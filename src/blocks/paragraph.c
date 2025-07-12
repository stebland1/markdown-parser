#include "blocks/paragraph.h"
#include "markdown.h"
#include "token.h"
#include "utils/stack.h"

int paragraph_block_start(ParserContext *ctx) {
  Token *paragraph = create_token(PARAGRAPH, PARAGRAPH_GROWTH_FACTOR, NULL);
  if (!paragraph) {
    return -1;
  }

  if (push(ctx->block_stack, &paragraph) < 0) {
    free_token(paragraph);
    return -1;
  }

  return 0;
}
