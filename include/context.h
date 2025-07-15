#ifndef CONTEXT_H
#define CONTEXT_H

#define INITIAL_DOCUMENT_SIZE 6

#include "token.h"
#include "utils/stack.h"

typedef struct {
  Stack block_stack;
  Token *ast;
  int in_front_matter;
} ParserContext;

void free_parser_context(ParserContext *ctx);
int init_parser_context(ParserContext *ctx);

#endif
