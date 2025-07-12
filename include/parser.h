#ifndef PARSER_H
#define PARSER_H

#include "token.h"
#include "utils/stack.h"
#include <stddef.h>
#include <stdio.h>

typedef struct {
  Stack *block_stack;
  Token *ast;
  int in_front_matter;
} ParserContext;

int parse_file(FILE *file, ParserContext *ctx);
int classify_line_type(char *line, ParserContext *ctx);

#endif
