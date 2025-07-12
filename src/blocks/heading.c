#include "blocks/heading.h"
#include "token.h"
#include <ctype.h>

int heading_block_start(char *line, ParserContext *ctx) {
  unsigned char level = 1;

  line++;
  while (*line == '#') {
    level++;
    line++;
  }

  while (isspace(*line))
    line++;

  Token *token = create_token(HEADING, 0, line);
  if (!token) {
    return -1;
  }

  token->level = level;

  if (add_child_to_token(ctx->ast, token)) {
    free_token(token);
    return -1;
  }

  return 0;
}
