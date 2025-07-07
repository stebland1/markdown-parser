#include "markdown.h"
#include "stack.h"
#include "token.h"
#include "utils.h"
#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FRONT_MATTER_DELIM "---"
#define PARAGRAPH_GROWTH_FACTOR 4
#define MAX_LINE 256

int process_file(FILE *file, Stack *block_stack, Stack *inline_stack,
                 Token *ast) {
  char line[MAX_LINE];
  int in_front_matter = 0;

  while (fgets(line, sizeof(line), file)) {
    if (is_front_matter(line, &in_front_matter) == 1)
      continue;
    line[strcspn(line, "\n")] = '\0';

    if (is_blank_line(line)) {
      if (handle_blank_line(block_stack, inline_stack, ast) < 0) {
        return -1;
      }
      continue;
    }

    if (*line == '#') {
      if (handle_heading(line, ast) < 0) {
        return -1;
      }
      continue;
    }

    Token **curblock_ptr = peek_stack(block_stack);
    if (!curblock_ptr) {
      return -1;
    }

    Token *curblock = *curblock_ptr;
    switch (curblock->type) {
    case PARAGRAPH:
      if (handle_inline(line, inline_stack) < 0) {
        return -1;
      }
      break;
    default: {
      if (handle_paragraph(line, block_stack) < 0) {
        return -1;
      }
      if (handle_inline(line, inline_stack) < 0) {
        return -1;
      }
      break;
    }
    }
  }

  return 0;
}

int is_front_matter(char *line, int *in_front_matter) {
  if (!*in_front_matter && strncmp(line, FRONT_MATTER_DELIM, 3) == 0) {
    *in_front_matter = 1;
    return 1;
  }

  if (*in_front_matter) {
    if (strncmp(line, FRONT_MATTER_DELIM, 3) == 0) {
      *in_front_matter = 0;
    }
    return 1;
  }

  return 0;
}

int handle_heading(char *line, Token *ast) {
  Token *heading = NULL;
  if (parse_heading(line, &heading) < 0) {
    return -1;
  }

  if (add_child_to_token(ast, heading)) {
    free(heading);
    return -1;
  }

  return 0;
}

int handle_inline(char *line, Stack *inline_stack) {
  Token **stack_top_ptr = peek_stack(inline_stack);
  if (!stack_top_ptr) {
    Token *text = create_token(TEXT, 0, line);
    return push(inline_stack, &text) < 0 ? -1 : 0;
  }

  Token *stack_top = *stack_top_ptr;
  size_t old_len = stack_top->content ? strlen(stack_top->content) : 0;
  size_t new_len = old_len + 1 + strlen(line) + 1;

  char *new_content = realloc(stack_top->content, new_len);
  if (!new_content) {
    return -1;
  }

  stack_top->content = new_content;
  stack_top->content[old_len] = ' ';
  strcpy(stack_top->content + old_len + 1, line);
  return 0;
}

int handle_paragraph(char *line, Stack *stack) {
  Token *paragraph = create_token(PARAGRAPH, PARAGRAPH_GROWTH_FACTOR, NULL);
  if (!paragraph) {
    return -1;
  }

  if (push(stack, &paragraph) < 0) {
    free_token(paragraph);
    return -1;
  }

  return 0;
}

int handle_blank_line(Stack *block_stack, Stack *inline_stack, Token *ast) {
  Token **parent_block_ptr = peek_stack(block_stack);
  if (!parent_block_ptr) {
    return -1;
  }

  Token *parent_block = *parent_block_ptr;
  // Ensures we don't pop the root AST node from the stack.
  if (parent_block->type == DOCUMENT) {
    return 0;
  }

  if (pop(block_stack, &parent_block) < 0) {
    return -1;
  }

  Stack reversed;
  // Reverse the inline stack to ensure correct ordering of inline nodes.
  if (reverse_stack(&reversed, inline_stack) < 0) {
    return -1;
  }

  while (!is_stack_empty(&reversed)) {
    Token *inline_child = NULL;
    pop(&reversed, &inline_child);

    if (add_child_to_token(parent_block, inline_child) < 0) {
      return -1;
    }
  }

  if (add_child_to_token(ast, parent_block) < 0) {
    return -1;
  }

  return 0;
}

int parse_heading(char *s, Token **out) {
  *out = NULL;

  unsigned char level = 1;
  s++;
  while (*s == '#') {
    level++;
    s++;
  }
  while (isspace(*s))
    s++;
  Token *token = create_token(HEADING, 0, s);
  if (!token) {
    return -1;
  }
  token->level = level;
  *out = token;

  return 0;
}

void print_ast(Token *root, int level) {
  if (!root) {
    return;
  }

  for (size_t i = 0; i < level; i++) {
    printf("\t");
  }

  char *type;
  switch (root->type) {
  case HEADING:
    type = "HEADING";
    break;
  case DOCUMENT:
    type = "DOCUMENT";
    break;
  case TEXT:
    type = "TEXT";
    break;
  case PARAGRAPH:
    type = "PARAGRAPH";
    break;
  }

  printf("- type: %s\n", type);

  for (size_t i = 0; i < level; i++) {
    printf("\t");
  }

  if (root->content) {
    printf("- content: %s\n", root->content);
  }

  for (size_t i = 0; i < level; i++) {
    printf("\t");
  }
  printf("-----------------------\n");

  for (size_t i = 0; i < root->child_count; i++) {
    print_ast(root->children[i], level + 1);
  }
}

void free_ast(Token *root) {
  for (size_t i = 0; i < root->child_count; i++) {
    free_ast(root->children[i]);
  }

  if (root->content) {
    free(root->content);
  }

  if (root->children) {
    free(root->children);
  }

  free(root);
}
