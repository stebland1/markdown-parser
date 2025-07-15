#include "blocks/stack.h"
#include "context.h"
#include "token.h"
#include "utils/stack.h"

int stop_at_non_paragraph(Token *token, void *_) {
  return token->type != PARAGRAPH;
}

int stop_at_non_list(Token *token, void *_) { return token->type != LIST; }

int flush_paragraph(ParserContext *ctx) {
  return flush_stack(ctx, stop_at_non_paragraph, NULL, NULL);
}

int flush_list(ParserContext *ctx) {
  return flush_stack(ctx, stop_at_non_list, NULL, NULL);
}

int flush_remaining_blocks(ParserContext *ctx) {
  return flush_stack(ctx, NULL, NULL, NULL);
}

int flush_stack(ParserContext *ctx, FlushPredicate stop_when,
                FlushCallback on_flush, void *userdata) {
  while (!is_stack_empty(&ctx->block_stack)) {
    Token *cur_block = peek_stack_value(&ctx->block_stack);

    // Ensures we don't pop the root AST node from the stack.
    if (cur_block->type == DOCUMENT) {
      return 0;
    }

    if (stop_when && stop_when(cur_block, userdata)) {
      return 0;
    }

    if (pop(&ctx->block_stack, &cur_block) < 0) {
      return -1;
    }

    if (on_flush) {
      on_flush(ctx, cur_block, userdata);
    } else if (add_child_to_token(ctx->ast, cur_block) < 0) {
      free_token(cur_block);
      return -1;
    }
  }

  return 0;
}
