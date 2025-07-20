#ifndef BLOCK_QUOTE_H
#define BLOCK_QUOTE_H

#include "context.h"

#define BLOCK_QUOTE_CHILD_GROWTH_FACTOR 4

int handle_block_quote(ParserContext *ctx, char *line);

#endif
