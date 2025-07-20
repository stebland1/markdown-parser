#include "blocks/block_quote.h"
#include "blocks/paragraph.h"
#include "context.h"
#include "inline/parser.h"
#include "token.h"
#include "utils/stack.h"
#include <ctype.h>

int handle_block_quote(ParserContext *ctx, char *line) {
  Token *active_block = peek_stack_value(&ctx->block_stack);
  if (active_block->type != BLOCK_QUOTE) {
    Token *block_quote_token =
        create_token(BLOCK_QUOTE, BLOCK_QUOTE_CHILD_GROWTH_FACTOR, NULL, NULL);
    if (!block_quote_token) {
      return -1;
    }

    if (push(&ctx->block_stack, &block_quote_token) < 0) {
      free_token(block_quote_token);
      return -1;
    }
  }

  Token *block_quote_token = peek_stack_value(&ctx->block_stack);
  if (!block_quote_token) {
    return -1;
  }

  Token *line_token = create_token(LINE, LINE_GROWTH_FACTOR, NULL, NULL);
  if (!line_token) {
    return -1;
  }

  char *p = line;
  p++;
  while (isspace(*p)) {
    p++;
  }

  if (parse_line(p, line_token) < 0) {
    free_token(line_token);
    return -1;
  }

  if (add_child_to_token(block_quote_token, line_token) < 0) {
    free_token(line_token);
    return -1;
  }

  return 0;
}
