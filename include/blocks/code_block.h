#ifndef CODE_BLOCK_H
#define CODE_BLOCK_H

#include "context.h"

void handle_code_block_line(ParserContext *ctx, char *line);
int handle_code_block_close(ParserContext *ctx);

#endif
