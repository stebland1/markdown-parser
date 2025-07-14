#include "front_matter.h"
#include "inline/element.h"
#include "inline/parser.h"
#include "inline/stack.h"
#include "utils/stack.h"
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int is_matching_inline_delim(void *item, void *userdata) {
  InlineElement *element = (InlineElement *)item;
  Delimiter *target = (Delimiter *)userdata;

  return element->type == DELIMITER &&
         element->delimiter.symbol == target->symbol &&
         element->delimiter.count == target->count;
}

TokenType get_emphasis_token_type(char symbol, int count) {
  TokenType token_type = UNKNOWN;
  switch (symbol) {
  case UNDERSCORE:
  case ASTERISK:
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
  Token *token = create_token(token_type, children_len, NULL, NULL);
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

int is_punctuation(char c) { return ispunct(c) || c == '\0'; }
int is_whitespace(char c) { return isspace(c) || c == '\0'; }

void classify_delimiter_runs(char *start, char *end, char *line, int *can_open,
                             int *can_close) {
  char before = start > line ? *(start - 1) : '\0';
  char after = *end;

  int after_is_whitespace = is_whitespace(after);
  int after_is_punctuation = is_punctuation(after);
  int before_is_whitespace = is_whitespace(before);
  int before_is_punctuation = is_punctuation(before);

  int is_left_flanking =
      !after_is_whitespace &&
      (!after_is_punctuation || before_is_whitespace || before_is_punctuation);

  int is_right_flanking =
      !before_is_whitespace &&
      (!before_is_punctuation || after_is_whitespace || before_is_punctuation);

  *can_open = is_left_flanking;
  *can_close = is_right_flanking;
}

char *handle_emphasis(InlineParserContext *ctx) {
  // If it's escaped, skip trying to parse.
  // Treat as a normal character.
  if (is_escaped(ctx->c, ctx->line)) {
    ctx->text_buf[ctx->text_buf_len++] = *ctx->c++;
    return 0;
  }

  char *start = ctx->c;
  char symbol = *ctx->c;
  int count = 1;

  ctx->c++;
  while (*ctx->c == symbol) {
    count++;
    ctx->c++;
  }

  if (count > MAX_DELIMITER_LEN) {
    memset(&ctx->text_buf[ctx->text_buf_len], symbol, count);
    ctx->text_buf_len += count;
    return 0;
  }

  char *end = ctx->c;
  int can_open = 0;
  int can_close = 0;
  classify_delimiter_runs(start, end, ctx->line, &can_open, &can_close);

  // If the delim can be an open and close simultaneously
  // we know for sure it's an invalid delimiter.
  // Same goes for if it can't be either, (open OR close).
  // Treat it as a normal char and continue.
  if ((can_close && can_open) || (!can_close && !can_open)) {
    memset(&ctx->text_buf[ctx->text_buf_len], symbol, count);
    ctx->text_buf_len += count;
    return 0;
  }

  Delimiter delimiter = {.symbol = symbol, .count = count};
  InlineElement *elem = create_inline_element(DELIMITER, &delimiter);
  if (!elem) {
    return NULL;
  }

  // If it can open push it immediately onto the stack.
  // The stack shouldn't ever contain closing delimiters.
  if (can_open) {
    if (flush_text_into_stack(ctx) < 0) {
      return NULL;
    }

    if (push_to_inline_stack(ctx->inline_stack, elem) < 0) {
      free_inline_element(elem);
      return NULL;
    }

    return 0;
  }

  // From here on we know that this is a potential closing delimiter.
  Delimiter *close_delim = &elem->delimiter;
  InlineElement *open_delim =
      find_stack(ctx->inline_stack, is_matching_inline_delim, close_delim);

  // If we cannot find the matching delimiter,
  // then treat this current delimiter as text and continue.
  if (!open_delim) {
    memset(&ctx->text_buf[ctx->text_buf_len], symbol, count);
    ctx->text_buf_len += count;

    if (flush_text_into_stack(ctx) < 0) {
      return NULL;
    }

    free_inline_element(elem);
    return 0;
  }

  free_inline_element(elem);

  if (flush_text_into_stack(ctx) < 0) {
    return NULL;
  }

  // From here on, it's known that there should be an emphasis token.
  // Everything between open_delim to close_delim (exclusive)
  // should be its children.
  size_t buf_len = 0;
  InlineElement *children_buf[MAX_CHLD_BUF_SIZE];

  if (pop_until_delimiter(children_buf, &buf_len, ctx->inline_stack,
                          open_delim) < 0) {
    return NULL;
  }
  TokenType token_type = get_emphasis_token_type(open_delim->delimiter.symbol,
                                                 open_delim->delimiter.count);
  assert(token_type != UNKNOWN);
  free_inline_element(open_delim);

  if (create_emphasis_token(token_type, children_buf, buf_len,
                            ctx->inline_stack) < 0) {
    return NULL;
  }

  return 0;
}
