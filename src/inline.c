#include "inline.h"
#include "emphasis.h"
#include "inline/element.h"
#include "inline/stack.h"
#include "token.h"
#include "utils/stack.h"
#include "utils/utils.h"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

  InlineElement *prev = peek_stack_value(inline_stack);

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

int parse_line(char *line, Token *line_token) {
  Stack inline_stack;
  if (create_stack(&inline_stack, sizeof(InlineElement *)) < 0) {
    return -1;
  }

  char text_buf[1024];
  size_t text_buf_len = 0;

  char *p = line;
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

  if (flush_text_buf(text_buf, &text_buf_len, &inline_stack) < 0) {
    return -1;
  }

  for (ssize_t i = 0; i < inline_stack.count; i++) {
    InlineElement *inline_element = ((InlineElement **)inline_stack.items)[i];
    if (inline_element->type == DELIMITER) {
      if (handle_unmatched_delimiter(inline_element, &inline_stack) < 0) {
        return -1;
      }
      continue;
    }

    if (add_child_to_token(line_token, inline_element->token) < 0) {
      return -1;
    }
  }

  return 0;
}
