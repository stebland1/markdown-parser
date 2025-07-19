#include "inline/element.h"
#include "inline/emphasis.h"
#include "inline/parser.h"
#include "inline/stack.h"
#include "inline/undo.h"
#include "token.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

InlineElement *process_delimiter(InlineParserContext *ctx) {
  int count = 0;
  while (*ctx->c == BACKTICK) {
    ctx->c++;
    count++;
  }

  Delimiter delimiter = {.symbol = BACKTICK, .count = count, .prefix = 0};
  InlineElement *open_code_span_delimiter =
      create_inline_element(DELIMITER, &delimiter);
  return open_code_span_delimiter;
}

int push_code_span_into_stack(InlineParserContext *ctx, char *content) {
  Token *code_span_token = create_token(CODE_SPAN, 0, content, NULL);
  if (!code_span_token) {
    return -1;
  }

  InlineElement *code_span_element =
      create_inline_element(TOKEN, code_span_token);
  if (!code_span_element) {
    free_token(code_span_token);
    return -1;
  }

  if (push_to_inline_stack(ctx->inline_stack, code_span_element) < 0) {
    free_inline_element(code_span_element);
    return -1;
  }

  return 0;
}

char *element_to_markdown(void *item) {
  InlineElement *element = (InlineElement *)item;
  switch (element->type) {
  case TOKEN:
    return token_to_markdown(element->token);
  case DELIMITER:
    return delimiter_to_markdown(&element->delimiter);
  }
}

int handle_code_span(InlineParserContext *ctx) {
  if (flush_text_into_stack(ctx) < 0) {
    return -1;
  }

  InlineElement *cur_delim = process_delimiter(ctx);
  if (!cur_delim) {
    return -1;
  }

  InlineElement *potential_open = find_stack(
      ctx->inline_stack, is_matching_inline_delim, &cur_delim->delimiter);
  if (!potential_open) {
    if (push_to_inline_stack(ctx->inline_stack, cur_delim) < 0) {
      free_inline_element(cur_delim);
      return -1;
    }

    return 0;
  }

  size_t num_elements = 0;
  InlineElement *elements_to_flatten[MAX_CHILD_BUF_SIZE];
  if (pop_until_delimiter(elements_to_flatten, &num_elements, ctx->inline_stack,
                          potential_open) < 0) {
    free_inline_element(cur_delim);
    return -1;
  }

  char *markdown = join_items_as_str((void **)elements_to_flatten, num_elements,
                                     element_to_markdown);
  if (!markdown) {
    return -1;
  }

  if (push_code_span_into_stack(ctx, markdown) < 0) {
    free_inline_element(cur_delim);
    return -1;
  }

  return 0;
}
