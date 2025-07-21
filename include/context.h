#ifndef CONTEXT_H
#define CONTEXT_H

#define INITIAL_DOCUMENT_SIZE 6
#define MAX_CODE_BLOCK_SIZE 1024

#include "token.h"
#include "utils/stack.h"

typedef enum {
  FRONT_MATTER_NONE,
  FRONT_MATTER_IN,
  FRONT_MATTER_DONE
} FrontMatterState;

typedef struct {
  Stack block_stack;
  Token *ast;
  FrontMatterState front_matter_state;
  struct CodeBlock {
    int parsing;
    char buf[MAX_CODE_BLOCK_SIZE];
    size_t buf_len;
  } code_block;
} ParserContext;

void free_parser_context(ParserContext *ctx);
int init_parser_context(ParserContext *ctx);

#endif
