#include "markdown.h"
#include "stack.h"
#include "token.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 256
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

  char line[MAX_LINE];
  int in_front_matter = 0;

  while (fgets(line, sizeof(line), file)) {
    if (is_front_matter(line, &in_front_matter) == 1)
      continue;
    line[strcspn(line, "\n")] = '\0';

    if (is_blank_line(line)) {
      if (handle_blank_line(&block_stack, ast) < 0) {
        goto fail;
      }
      continue;
    }

    if (*line == '#') {
      if (handle_heading(line, ast) < 0) {
        goto fail;
      }
      continue;
    }

    Token **curblock_ptr = peek_stack(&block_stack);
    if (!curblock_ptr) {
      goto fail;
    }

    Token *curblock = *curblock_ptr;
    switch (curblock->type) {
    case PARAGRAPH:
      // handle inline elements.
      break;
    default: {
      if (handle_paragraph(line, &block_stack) < 0) {
        goto fail;
      }
      break;
    }
    }
  }

  while (!is_stack_empty(&block_stack)) {
    Token *top_ptr = NULL;
    if (pop(&block_stack, &top_ptr) < 0) {
      goto fail;
    }

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
