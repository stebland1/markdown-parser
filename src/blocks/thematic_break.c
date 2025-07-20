#include "context.h"
#include "token.h"
#include "utils/stack.h"

int handle_thematic_break(ParserContext *ctx) {
  Token *thematic_break_token = create_token(THEMATIC_BREAK, 0, NULL, NULL);
  if (!thematic_break_token) {
    return -1;
  }

  Token *active_block = peek_stack_value(&ctx->block_stack);
  if (add_child_to_token(active_block, thematic_break_token) < 0) {
    free_token(thematic_break_token);
    return -1;
  }

  return 0;
}
