#ifndef PARAGRAPH_H
#define PARAGRAPH_H

#include "context.h"

#define PARAGRAPH_GROWTH_FACTOR 4
#define LINE_GROWTH_FACTOR 6

int paragraph_block_start(ParserContext *ctx);
int handle_paragraph_line(ParserContext *ctx, char *line);

#endif
