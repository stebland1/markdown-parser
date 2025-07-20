#include "blocks/stack.h"
#include "context.h"
#include "token.h"
#include "utils/stack.h"

int stop_at_non_type(Token *token, void *userdata) {
  TokenType *expected_type = (TokenType *)userdata;
  return token->type != *expected_type;
}

int stop_at_non_list(Token *token, void *_) {
  return token->type != LIST && token->type != ORDERED_LIST;
}

int flush_paragraph(ParserContext *ctx) {
  TokenType type = PARAGRAPH;
  return flush_stack(ctx, stop_at_non_type, NULL, &type);
}

int add_parent_list_only(ParserContext *ctx, Token *token, void *_) {
  if ((token->type == LIST || token->type == ORDERED_LIST) &&
      token->meta->list.parent == 0) {
    return 0;
  }

  return add_child_to_token(ctx->ast, token);
}

int flush_list(ParserContext *ctx) {
  return flush_stack(ctx, stop_at_non_list, add_parent_list_only, NULL);
}

int flush_remaining_blocks(ParserContext *ctx) {
  return flush_stack(ctx, NULL, add_parent_list_only, NULL);
}

int flush_block_quote(ParserContext *ctx) {
  TokenType type = BLOCK_QUOTE;
  return flush_stack(ctx, stop_at_non_type, NULL, &type);
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
