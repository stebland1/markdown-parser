#include "inline.h"
#include "emphasis.h"
#include "stack.h"
#include "token.h"
#include "utils.h"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

InlineElement *create_inline_element(InlineElementType type, void *data) {
  InlineElement *elem = malloc(sizeof(InlineElement));
  if (!elem) {
    return NULL;
  }

  elem->type = type;

  switch (elem->type) {
  case TOKEN:
    elem->token = data;
    break;
  case DELIMITER:
    elem->delimiter = *(Delimiter *)data;
    break;
  default:
    free(elem);
    return NULL;
  }

  return elem;
}

void free_inline_element(InlineElement *elem) {
  if (!elem) {
    return;
  }

  switch (elem->type) {
  case TOKEN:
    free_token(elem->token);
    break;
  case DELIMITER:
    // Nothing to free
    break;
  }

  free(elem);
}

// Before pushing to the inline stack, we need to append prev and next
int push_to_inline_stack(Stack *inline_stack, InlineElement *element) {
  InlineElement **prev_ptr = peek_stack(inline_stack);
  if (prev_ptr) {
    InlineElement *prev_item = *prev_ptr;
    element->prev = prev_item;
    prev_item->next = element;
  }

  if (push(inline_stack, &element) < 0) {
    return -1;
  }

  return 0;
}

int concat_with_prev_token(char *line, InlineElement *prev) {
  char *new_content = concat(2, prev->token->content, line);
  if (!new_content) {
    return -1;
  }

  free(prev->token->content);
  prev->token->content = new_content;
  return 0;
}

int flush_text_buf(char *buf, size_t *len, Stack *inline_stack) {
  if (*len == 0) {
    return 0;
  }

  buf[*len] = '\0';
  *len = 0;

  InlineElement **prev_ptr = peek_stack(inline_stack);
  InlineElement *prev = (prev_ptr ? *prev_ptr : NULL);

  // Try to concatenate with the prev text token (if there is one).
  if (prev && prev->type == TOKEN && prev->token->type == TEXT) {
    if (concat_with_prev_token(buf, prev) < 0) {
      return -1;
    }

    return 0;
  }

  Token *token = create_token(TEXT, 0, buf);
  if (!token) {
    return -1;
  }

  InlineElement *elem = create_inline_element(TOKEN, token);
  if (!elem) {
    free_token(token);
    return -1;
  }

  if (push_to_inline_stack(inline_stack, elem) < 0) {
    free_inline_element(elem);
    return -1;
  }

  return 0;
}

int is_matching_delimiter(Delimiter *delim1, Delimiter *delim2) {
  if (delim1->symbol != delim2->symbol) {
    return -1;
  }

  if (delim1->count != delim2->count) {
    return -1;
  }

  return 0;
}

InlineElement *find_open_delimiter(Stack *inline_stack,
                                   Delimiter *close_delim) {
  InlineElement *open_delim = NULL;
  for (ssize_t i = inline_stack->count - 1; i >= 0; i--) {
    InlineElement *item = NULL;
    memcpy(&item, (char *)inline_stack->items + i * inline_stack->item_size,
           sizeof(InlineElement *));

    if (item->type == DELIMITER &&
        is_matching_delimiter(&item->delimiter, close_delim) == 0) {
      open_delim = item;
      break;
    }
  }

  return open_delim;
}

// check if the cur character is escaped i.e. followed by a backslash.
// return 1 for true 0 for false.
int is_escaped(char *c, char *line) {
  if (c <= line) {
    return 0;
  }

  return *(c - 1) == '\\';
}

int handle_unmatched_delimiter(InlineElement *delimiter, Stack *inline_stack) {
  InlineElement *prev = delimiter->prev;
  InlineElement *next = delimiter->next;

  char buf[MAX_DELIMITER_LEN];
  memset(buf, delimiter->delimiter.symbol, delimiter->delimiter.count);

  if (prev && next) {
    char *new_content =
        concat(3, prev->token->content, buf, next->token->content);
    if (!new_content) {
      return -1;
    }
    free(prev->token->content);
    prev->token->content = new_content;

    // pop the `next` item as it's now been concatenated.
    free_inline_element(delimiter);
    InlineElement *next_in_stack = NULL;
    pop(inline_stack, &next_in_stack);
    free_inline_element(next_in_stack);
    return 0;
  }

  if (prev) {
    char *new_content = concat(2, prev->token->content, buf);
    free_inline_element(delimiter);
    if (!new_content) {
      return -1;
    }
    free(prev->token->content);
    prev->token->content = new_content;
    return 0;
  }

  if (next) {
    char *new_content = concat(2, buf, next->token->content);
    free_inline_element(delimiter);
    if (!new_content) {
      return -1;
    }
    free(next->token->content);
    next->token->content = new_content;
    return 0;
  }

  return 0;
}

int parse_line(char *line, Stack *line_elements) {
  Stack inline_stack;
  if (create_stack(&inline_stack, sizeof(InlineElement *)) < 0) {
    return -1;
  }

  char text_buf[1024];
  size_t text_buf_len = 0;

  char *p = line;

  // iterate over every character in the line.
  // and stop when we hit the end.
  while (*p) {
    switch (*p) {
    case '*':
    case '_': {
      char *new_ptr =
          handle_emphasis(p, line, text_buf, &text_buf_len, &inline_stack);
      if (!new_ptr) {
        return -1;
      }
      p = new_ptr;
      break;
    }
    default:
      text_buf[text_buf_len++] = *p++;
      break;
    }
  }

  // Everything from inline, should make its way into line elements.
  // at this point, we should flush out the text_buf, into it's own TEXT node.
  if (flush_text_buf(text_buf, &text_buf_len, &inline_stack) < 0) {
    return -1;
  }

  Token **cur_line_ptr = peek_stack(line_elements);
  Token *cur_line = (cur_line_ptr ? *cur_line_ptr : NULL);

  Stack reversed_inline_stack;
  if (reverse_stack(&reversed_inline_stack, &inline_stack)) {
    return -1;
  }

  while (!is_stack_empty(&reversed_inline_stack)) {
    InlineElement *cur = NULL;
    pop(&reversed_inline_stack, &cur);

    assert(cur != NULL);

    if (cur->type == DELIMITER) {
      if (handle_unmatched_delimiter(cur, &reversed_inline_stack) < 0) {
        return -1;
      }
      continue;
    }

    if (add_child_to_token(cur_line, cur->token) < 0) {
      return -1;
    }
  }

  return 0;
}
