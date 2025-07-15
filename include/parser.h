#ifndef PARSER_H
#define PARSER_H

#include "context.h"
#include <stddef.h>
#include <stdio.h>

int parse_file(FILE *file, ParserContext *ctx);
int classify_line_type(char *line, ParserContext *ctx);
int flush_current_active_block(ParserContext *ctx);

#endif
