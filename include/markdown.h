#ifndef MARKDOWN_H
#define MARKDOWN_H

#include "token.h"
#include "utils/stack.h"
#include <stddef.h>
#include <stdio.h>

typedef struct {
  Stack *block_stack;
  Token *ast;
  int in_front_matter;
} ParserContext;

int process_file(FILE *file, ParserContext *ctx);
int is_front_matter(char *line, ParserContext *ctx);

#endif
