#include "parser.h"
#include "utils/stack.h"

int blank_line_block_start(ParserContext *ctx) {
  Token *active_block = peek_stack_value(ctx->block_stack);
  // Ensures we don't pop the root AST node from the stack.
  if (active_block->type == DOCUMENT) {
    return 0;
  }

  if (pop(ctx->block_stack, &active_block) < 0) {
    return -1;
  }

  if (add_child_to_token(ctx->ast, active_block) < 0) {
    return -1;
  }

  return 0;
}
