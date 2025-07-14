#include "inline/parser.h"
#include "inline/element.h"
#include "inline/emphasis.h"
#include "inline/link.h"
#include "inline/stack.h"
#include "token.h"
#include "utils/stack.h"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// check if the cur character is escaped i.e. followed by a backslash.
// return 1 for true 0 for false.
int is_escaped(char *c, char *line) {
  if (c <= line) {
    return 0;
  }

  return *(c - 1) == '\\';
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
    case OPEN_SQUARE_BRACKET: {
      Delimiter delimiter = {.symbol = OPEN_SQUARE_BRACKET, .count = 1};
      InlineElement *open_link_delimiter_element =
          create_inline_element(DELIMITER, &delimiter);

      if (push_to_inline_stack(&inline_stack, open_link_delimiter_element) <
          0) {
        return -1;
      }
      p++;
      break;
    };
    case CLOSE_SQUARE_BRACKET: {
      char *new_ptr = parse_link(p, &inline_stack, text_buf, &text_buf_len);
      if (!new_ptr) {
        return -1;
      }
      p = new_ptr;
      break;
    }
    case ASTERISK:
    case UNDERSCORE: {
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

  if (flush_text_into_stack(text_buf, &text_buf_len, &inline_stack) < 0) {
    return -1;
  }

  for (ssize_t i = 0; i < inline_stack.count; i++) {
    InlineElement *inline_element = ((InlineElement **)inline_stack.items)[i];
    if (inline_element->type == DELIMITER) {
      if (merge_unmatched_delimiters(inline_element, &inline_stack) < 0) {
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
