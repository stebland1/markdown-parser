#include "markdown.h"
#include "stack.h"
#include "token.h"
#include <stdio.h>
#include <stdlib.h>

#define INITIAL_DOCUMENT_SIZE 6

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

  Token *ast = create_token(DOCUMENT, INITIAL_DOCUMENT_SIZE, NULL);
  if (!ast) {
    return EXIT_FAILURE;
  }

  Stack block_stack;
  if (create_stack(&block_stack, sizeof(Token *)) < 0) {
    free_token(ast);
    fclose(file);
    return EXIT_FAILURE;
  }

  Token *doc_ptr = ast;
  if (push(&block_stack, &doc_ptr) < 0) {
    goto fail;
  }

  if (process_file(file, &block_stack, ast) < 0) {
    goto fail;
  }

  // flush what's left in the stack
  while (!is_stack_empty(&block_stack)) {
    Token *top_ptr = NULL;
    if (pop(&block_stack, &top_ptr) < 0) {
      goto fail;
    }

    // don't attach the root node to itself.
    if (top_ptr->type == DOCUMENT) {
      break;
    }

    if (add_child_to_token(ast, top_ptr) < 0) {
      goto fail;
    }
  }

  print_ast(ast, 0);

  free_token(ast);
  free_stack(&block_stack);
  fclose(file);
  return EXIT_SUCCESS;

fail:
  free_token(ast);
  free_stack(&block_stack);
  fclose(file);
  return EXIT_FAILURE;
}
