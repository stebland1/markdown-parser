#include "context.h"
#include "parser.h"
#include "token.h"
#include "utils/debug.h"
#include "utils/stack.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "Invalid argument count: provided %d, expected 1\n",
            argc - 1);
    return EXIT_FAILURE;
  }

  char *path = argv[1];
  FILE *file = fopen(path, "r");

  if (file == NULL) {
    fprintf(stderr, "Failed to open file: %s\n", path);

    return EXIT_FAILURE;
  }

  ParserContext ctx;
  if (init_parser_context(&ctx) < 0) {
    fclose(file);
    return EXIT_FAILURE;
  }

  if (parse_file(file, &ctx) < 0) {
    goto fail;
  }

  // flush what's left in the stack
  while (!is_stack_empty(&ctx.block_stack)) {
    Token *top_ptr = NULL;
    if (pop(&ctx.block_stack, &top_ptr) < 0) {
      goto fail;
    }

    // don't attach the root node to itself.
    if (top_ptr->type == DOCUMENT) {
      break;
    }

    if (add_child_to_token(ctx.ast, top_ptr) < 0) {
      goto fail;
    }
  }

  print_ast(ctx.ast, 0);

  free_parser_context(&ctx);
  fclose(file);
  return EXIT_SUCCESS;

fail:
  free_parser_context(&ctx);
  fclose(file);
  return EXIT_FAILURE;
}
