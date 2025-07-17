#include "blocks/code_block.h"
#include "context.h"
#include "parser.h"
#include "utils/utils.h"
#include <string.h>

void handle_code_block_line(ParserContext *ctx, char *line) {
  size_t len = strlen(line);
  char tmp[MAX_LINE + 1];
  memcpy(tmp, line, len);

  tmp[len++] = '\n';
  tmp[len] = '\0';

  size_t available_space = MAX_CODE_BLOCK_SIZE - ctx->code_block.buf_len - 1;
  size_t n = MIN(available_space, len);
  memcpy(ctx->code_block.buf + ctx->code_block.buf_len, tmp, n);
  ctx->code_block.buf_len += n;
  ctx->code_block.buf[ctx->code_block.buf_len] = '\0';
}

int handle_code_block_close(ParserContext *ctx) {
  Token *code_block_token =
      create_token(CODE_BLOCK, 0, ctx->code_block.buf, NULL);
  if (!code_block_token) {
    return -1;
  }

  if (add_child_to_token(ctx->ast, code_block_token) < 0) {
    free_token(code_block_token);
    return -1;
  }

  ctx->code_block.buf[0] = '\0';
  ctx->code_block.buf_len = 0;
  return 0;
}
