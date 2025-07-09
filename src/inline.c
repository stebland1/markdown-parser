#include "inline.h"
#include "markdown.h"
#include "stack.h"
#include "token.h"
#include "utils.h"
#include <assert.h>
#include <ctype.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int flush_text_buf(char *buf, size_t *len, Stack *stack) {
  if (*len == 0) {
    return 0;
  }

  buf[*len] = '\0';
  *len = 0;

  return handle_text(buf, stack) < 0 ? -1 : 0;
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

// check if the cur char can be interpreted as an opening emphasis tag
// such as '*' or '_'
int can_open_emphasis(char *c, char *line) {
  return (c == line) || isspace(*(c - 1)) || ispunct(*(c - 1));
}

int can_close_emphasis(char *c, char *line) {
  return (*c == '\0') || isspace(*c) || ispunct(*c);
}

TokenType get_emphasis_token_type(char symbol, int count) {
  TokenType token_type = UNKNOWN;
  switch (symbol) {
  case '_':
  case '*':
    switch (count) {
    case 2:
      token_type = BOLD;
      break;
    case 1:
      token_type = ITALIC;
      break;
    }
    break;
  }

  return token_type;
}

char *handle_emphasis(char *c, char *line, char *text_buf, size_t *text_buf_len,
                      Stack *inline_stack) {
  // If it's escaped, skip trying to parse.
  // Treat as a normal character.
  if (is_escaped(c, line)) {
    text_buf[(*text_buf_len)++] = *c++;
    return c;
  }

  char symbol = *c;
  int count = 1;
  int can_open = can_open_emphasis(c, line);

  c++;
  while (*c == symbol) {
    count++;
    c++;
  }

  // If the delim can be an open and close simultaneously
  // we know for sure it's an invalid delimiter.
  // Treat it as a normal char and continue.
  if (can_close_emphasis(c, line) && can_open) {
    while (count--) {
      text_buf[(*text_buf_len)++] = *c++;
    }
    return c;
  }

  // From here on, we know we have a potential valid delimiter.
  // So flush the text buffer.
  if (flush_text_buf(text_buf, text_buf_len, inline_stack) < 0) {
    return c;
  }

  InlineElement *elem = malloc(sizeof(InlineElement));
  if (!elem) {
    return NULL;
  }

  elem->type = DELIMITER;
  elem->delimiter.symbol = symbol;
  elem->delimiter.count = count;

  // If it can open push it immediately onto the stack.
  // The stack shouldn't ever contain closing delimiters.
  if (can_open) {
    if (push(inline_stack, &elem) < 0) {
      free(elem);
      return NULL;
    }

    return c;
  }

  // From here on we know that this is a potential closing delimiter.
  Delimiter *close_delim = &elem->delimiter;
  InlineElement *open_delim = find_open_delimiter(inline_stack, close_delim);

  // If we cannot find the matching delimiter,
  // then treat this current delimiter as text and continue.
  if (!open_delim) {
    for (size_t i = 0; i < close_delim->count; i++) {
      text_buf[(*text_buf_len)++] = close_delim->symbol;
    }

    free(elem);
    return c;
  }

  free(elem);

  // From here on, it's known that there should be an emphasis token.
  // Everything between open_delim to close_delim (exclusive)
  // should be its children.
  size_t buf_len = 0;
  InlineElement *children_buf[64];
  InlineElement *cur = NULL;

  do {
    if (pop(inline_stack, &cur) < 0) {
      for (size_t i = 0; i < buf_len; i++) {
        free(children_buf[i]);
      }
      return NULL;
    }

    if (cur != open_delim) {
      children_buf[buf_len++] = cur;
    }
  } while (cur != open_delim);

  reverse_list(children_buf, buf_len, sizeof(InlineElement *));

  TokenType token_type = get_emphasis_token_type(open_delim->delimiter.symbol,
                                                 open_delim->delimiter.count);
  assert(token_type != UNKNOWN);
  free(open_delim);

  // Create the emphasis token
  Token *token = create_token(token_type, buf_len, NULL);
  for (size_t i = 0; i < buf_len; i++) {
    if (add_child_to_token(token, children_buf[i]->token) < 0) {
      free_token(token);
      return NULL;
    }
  }

  InlineElement *element = malloc(sizeof(InlineElement));
  if (!element) {
    free_token(token);
    return NULL;
  }

  element->type = TOKEN;
  element->token = token;
  if (push(inline_stack, &element) < 0) {
    free_token(token);
    return NULL;
  }

  return c;
}

int parse_line(char *line, Stack *inline_stack) {
  char text_buf[1024];
  size_t text_buf_len = 0;

  char *p = line;
  while (*p) {
    switch (*p) {
    case '*':
    case '_': {
      char *new_ptr =
          handle_emphasis(p, line, text_buf, &text_buf_len, inline_stack);
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

  return flush_text_buf(text_buf, &text_buf_len, inline_stack);
}
