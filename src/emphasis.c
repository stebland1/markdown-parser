#include "front_matter.h"
#include "inline.h"
#include "utils/stack.h"
#include "utils/utils.h"
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

int create_emphasis_token(TokenType token_type, InlineElement **children,
                          size_t children_len, Stack *inline_stack) {
  Token *token = create_token(token_type, children_len, NULL);
  for (size_t i = 0; i < children_len; i++) {
    if (add_child_to_token(token, children[i]->token) < 0) {
      free_token(token);
      return -1;
    }
  }

  InlineElement *element = create_inline_element(TOKEN, token);
  if (!element) {
    free_token(token);
    return -1;
  }

  if (push_to_inline_stack(inline_stack, element) < 0) {
    free_token(token);
    return -1;
  }

  return 0;
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

  if (count > MAX_DELIMITER_LEN) {
    memset(&text_buf[*text_buf_len], symbol, count);
    *text_buf_len += count;
    return c;
  }

  int can_close = can_close_emphasis(c, line);

  // If the delim can be an open and close simultaneously
  // we know for sure it's an invalid delimiter.
  // Same goes for if it can't be either, (open OR close).
  // Treat it as a normal char and continue.
  if ((can_close && can_open) || (!can_close && !can_open)) {
    memset(&text_buf[*text_buf_len], symbol, count);
    *text_buf_len += count;
    return c;
  }

  Delimiter delimiter = {.symbol = symbol, .count = count};
  InlineElement *elem = create_inline_element(DELIMITER, &delimiter);
  if (!elem) {
    return NULL;
  }

  // If it can open push it immediately onto the stack.
  // The stack shouldn't ever contain closing delimiters.
  if (can_open) {
    if (flush_text_buf(text_buf, text_buf_len, inline_stack) < 0) {
      return NULL;
    }

    if (push_to_inline_stack(inline_stack, elem) < 0) {
      free_inline_element(elem);
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
    memset(&text_buf[*text_buf_len], symbol, count);
    *text_buf_len += count;

    if (flush_text_buf(text_buf, text_buf_len, inline_stack) < 0) {
      return NULL;
    }

    free_inline_element(elem);
    return c;
  }

  free_inline_element(elem);

  if (flush_text_buf(text_buf, text_buf_len, inline_stack) < 0) {
    return NULL;
  }

  // From here on, it's known that there should be an emphasis token.
  // Everything between open_delim to close_delim (exclusive)
  // should be its children.
  size_t buf_len = 0;
  InlineElement *children_buf[64];
  InlineElement *cur = NULL;

  do {
    if (pop(inline_stack, &cur) < 0) {
      for (size_t i = 0; i < buf_len; i++) {
        free_inline_element(children_buf[i]);
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
  free_inline_element(open_delim);

  if (create_emphasis_token(token_type, children_buf, buf_len, inline_stack) <
      0) {
    return NULL;
  }

  return c;
}
