#include "inline/element.h"
#include "inline/emphasis.h"
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

// check if the cur character is escaped i.e. followed by a backslash.
// return 1 for true 0 for false.
int is_escaped(char *c, char *line) {
  if (c <= line) {
    return 0;
  }

  return *(c - 1) == '\\';
}

int is_open_link_delimiter(void *item, void *userdata) {
  InlineElement *element = (InlineElement *)item;
  return element->type == DELIMITER && element->delimiter.symbol == '[';
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
    case '[': {
      Delimiter delimiter = {.symbol = '[', .count = 1};
      InlineElement *open_link_delimiter_element =
          create_inline_element(DELIMITER, &delimiter);

      if (push_to_inline_stack(&inline_stack, open_link_delimiter_element) <
          0) {
        return -1;
      }
      p++;
      break;
    };
    case ']': {
      InlineElement *matching_delimiter =
          find_stack(&inline_stack, is_open_link_delimiter, NULL);
      if (!matching_delimiter) {
        text_buf[text_buf_len++] = *p++;
        continue;
      }

      if (*(p + 1) == '\0' || *(p + 1) != '(') {
        text_buf[text_buf_len++] = *p++;
        continue;
      }

      if (flush_text_into_stack(text_buf, &text_buf_len, &inline_stack) < 0) {
        return -1;
      }

      p += 2;
      while (*p && *p != ')') {
        text_buf[text_buf_len++] = *p;
        p++;
      }

      if (*p == '\0') {
        continue;
      }

      p++;
      text_buf[text_buf_len] = '\0';
      text_buf_len = 0;
      Token *token = create_token(LINK, 1, text_buf, NULL);
      create_inline_element(TOKEN, token);

      size_t buf_len = 0;
      InlineElement *children_buf[64];
      InlineElement *cur = NULL;

      do {
        if (pop(&inline_stack, &cur) < 0) {
          for (size_t i = 0; i < buf_len; i++) {
            free_inline_element(children_buf[i]);
          }
          return -1;
        }

        if (cur != matching_delimiter) {
          children_buf[buf_len++] = cur;
        }
      } while (cur != matching_delimiter);

      reverse_list(children_buf, buf_len, sizeof(InlineElement *));
      free_inline_element(matching_delimiter);

      for (ssize_t i = 0; i < buf_len; i++) {
        if (add_child_to_token(token, children_buf[i]->token) < 0) {
          free_token(token);
          return -1;
        }
      }

      InlineElement *link_element = create_inline_element(TOKEN, token);
      if (push_to_inline_stack(&inline_stack, link_element) < 0) {
        return -1;
      }
      break;
    }
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
