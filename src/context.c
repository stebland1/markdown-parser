#include "context.h"
#include "token.h"
#include "utils/stack.h"

int init_parser_context(ParserContext *ctx) {
  Token *ast = create_token(DOCUMENT, INITIAL_DOCUMENT_SIZE, NULL, NULL);
  if (!ast) {
    return -1;
  }

  Stack block_stack;
  if (create_stack(&block_stack, sizeof(Token *)) < 0) {
    free_token(ast);
    return -1;
  }

  Token *doc_ptr = ast;
  if (push(&block_stack, &doc_ptr) < 0) {
    free_parser_context(ctx);
    return -1;
  }

  ctx->block_stack = block_stack;
  ctx->ast = ast;
  ctx->in_front_matter = 0;
  ctx->code_block.parsing = 0;
  ctx->code_block.buf[0] = '\0';
  ctx->code_block.buf_len = 0;

  return 0;
}

void free_parser_context(ParserContext *ctx) {
  free_token(ctx->ast);
  free_stack(&ctx->block_stack);
}
