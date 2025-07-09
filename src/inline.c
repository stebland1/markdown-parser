#include "inline.h"
#include "markdown.h"
#include "stack.h"
#include "token.h"
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

int parse_line(char *line, Stack *inline_stack) {
  char text_buf[1024];
  size_t text_buf_len = 0;

  char *p = line;
  while (*p) {
    switch (*p) {
    case '*':
    case '_': {
      if (is_escaped(p, line)) {
        text_buf[text_buf_len++] = *p++;
        continue;
      }

      char symbol = *p;
      int count = 1;
      int can_open = can_open_emphasis(p, line);

      p++;
      while (*p == symbol) {
        count++;
        p++;
      }

      if (can_close_emphasis(p, line) && can_open) {
        while (count--) {
          text_buf[text_buf_len++] = *p++;
        }
        continue;
      }

      if (flush_text_buf(text_buf, &text_buf_len, inline_stack) < 0) {
        return -1;
      }

      InlineElement *elem = malloc(sizeof(InlineElement));
      if (!elem) {
        return -1;
      }

      elem->type = DELIMITER;
      elem->delimiter.symbol = symbol;
      elem->delimiter.count = count;

      if (can_open) {
        if (push(inline_stack, &elem) < 0) {
          free(elem);
          return -1;
        }
        continue;
      }

      Delimiter *close_delim = &elem->delimiter;

      InlineElement *target = find_open_delimiter(inline_stack, close_delim);

      if (!target) {
        for (size_t i = 0; i < close_delim->count; i++) {
          text_buf[text_buf_len++] = close_delim->symbol;
        }
        free(elem);
        continue;
      }

      InlineElement *emphasis_children_buf[64];
      size_t buf_len = 0;

      InlineElement *stack_top;
      do {
        if (pop(inline_stack, &stack_top) < 0) {
          return -1;
        }

        if (stack_top != target) {
          emphasis_children_buf[buf_len++] = stack_top;
        }
      } while (stack_top != target);

      for (size_t i = 0; i < buf_len / 2; i++) {
        InlineElement *tmp = emphasis_children_buf[i];
        emphasis_children_buf[i] = emphasis_children_buf[buf_len - 1 - i];
        emphasis_children_buf[buf_len - 1 - i] = tmp;
      }

      TokenType token_type;
      switch (target->delimiter.symbol) {
      case '_':
      case '*':
        switch (target->delimiter.count) {
        case 2:
          token_type = BOLD;
          break;
        case 1:
          token_type = ITALIC;
          break;
        }
      }

      free(target);

      Token *token = create_token(token_type, buf_len, NULL);
      for (size_t i = 0; i < buf_len; i++) {
        if (add_child_to_token(token, emphasis_children_buf[i]->token) < 0) {
          free_token(token);
          return -1;
        }
      }

      InlineElement *element = malloc(sizeof(InlineElement));
      if (!element) {
        free_token(token);
        return -1;
      }

      element->type = TOKEN;
      element->token = token;
      if (push(inline_stack, &element) < 0) {
        free_token(token);
        return -1;
      }
      break;
    }
    default:
      text_buf[text_buf_len++] = *p++;
      break;
    }
  }

  return flush_text_buf(text_buf, &text_buf_len, inline_stack);
}
