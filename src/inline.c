#include "inline.h"
#include "emphasis.h"
#include "markdown.h"
#include "stack.h"
#include "token.h"
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
