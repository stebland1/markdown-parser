#include "inline/element.h"
#include "inline/emphasis.h"
#include "inline/parser.h"
#include "inline/stack.h"
#include "token.h"
#include <stddef.h>

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

int push_code_span_into_stack(InlineParserContext *ctx,
                              InlineElement **children, size_t child_count) {
  Token *code_span_token = create_token(CODE_SPAN, child_count, NULL, NULL);
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

  size_t buf_len = 0;
  InlineElement *children_buf[MAX_CHLD_BUF_SIZE];
  // TODO: When grabbing children, any emphasis tags, need turning into text.
  // So I think we need to recursively grab all of the TEXT tokens and
  // concatenate them.
  if (pop_until_delimiter(children_buf, &buf_len, ctx->inline_stack,
                          potential_open) < 0) {
    free_inline_element(cur_delim);
    return -1;
  }

  if (push_code_span_into_stack(ctx, children_buf, buf_len) < 0) {
    free_inline_element(cur_delim);
    return -1;
  }

  return 0;
}
