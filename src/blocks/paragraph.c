#include "blocks/paragraph.h"
#include "blocks/stack.h"
#include "context.h"
#include "inline/parser.h"
#include "token.h"
#include "utils/stack.h"
#include <assert.h>

int paragraph_block_start(ParserContext *ctx) {
  Token *paragraph =
      create_token(PARAGRAPH, PARAGRAPH_GROWTH_FACTOR, NULL, NULL);
  if (!paragraph) {
    return -1;
  }

  if (push(&ctx->block_stack, &paragraph) < 0) {
    free_token(paragraph);
    return -1;
  }

  return 0;
}

int handle_paragraph_line(ParserContext *ctx, char *line) {
  Token *active_block = peek_stack_value(&ctx->block_stack);
  if (active_block->type != PARAGRAPH) {
    if (flush_list(ctx) < 0) {
      return -1;
    }
    if (paragraph_block_start(ctx) < 0) {
      return -1;
    }
  }

  active_block = peek_stack_value(&ctx->block_stack);
  assert(active_block != NULL);
  assert(active_block->type != DOCUMENT);

  Token *line_tok = create_token(LINE, LINE_GROWTH_FACTOR, NULL, NULL);
  if (!line_tok) {
    return -1;
  }

  if (parse_line(line, line_tok) < 0) {
    free_token(line_tok);
    return -1;
  }

  if (add_child_to_token(active_block, line_tok) < 0) {
    return -1;
  }

  return 0;
}
