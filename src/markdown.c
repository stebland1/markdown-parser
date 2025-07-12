#include "markdown.h"
#include "inline.h"
#include "stack.h"
#include "token.h"
#include "utils.h"
#include <assert.h>
#include <ctype.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FRONT_MATTER_DELIM "---"
#define PARAGRAPH_GROWTH_FACTOR 4
#define LINE_GROWTH_FACTOR 6
#define MAX_LINE 256

int handle_paragraph(Stack *stack) {
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

int process_file(FILE *file, Stack *block_stack, Stack *line_elements,
                 Token *ast) {
  char line[MAX_LINE];
  int in_front_matter = 0;

  while (fgets(line, sizeof(line), file)) {
    if (is_front_matter(line, &in_front_matter) == 1)
      continue;
    line[strcspn(line, "\n")] = '\0';

    if (is_blank_line(line)) {
      if (handle_blank_line(block_stack, line_elements, ast) < 0) {
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
    case PARAGRAPH: {
      Token *line_tok = create_token(LINE, LINE_GROWTH_FACTOR, NULL);
      if (!line_tok) {
        free_stack(line_elements);
        return -1;
      }

      if (push(line_elements, &line_tok) < 0) {
        free(line_tok);
        free_stack(line_elements);
        return -1;
      }

      if (parse_line(line, line_elements) < 0) {
        return -1;
      }
      break;
    }
    default: {
      if (handle_paragraph(block_stack) < 0) {
        return -1;
      }

      Token *line_tok = create_token(LINE, LINE_GROWTH_FACTOR, NULL);
      if (!line_tok) {
        free_stack(line_elements);
        return -1;
      }

      if (push(line_elements, &line_tok) < 0) {
        free(line_tok);
        free_stack(line_elements);
        return -1;
      }

      if (parse_line(line, line_elements) < 0) {
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

int handle_blank_line(Stack *block_stack, Stack *line_elements, Token *ast) {
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
  if (reverse_stack(&reversed, line_elements) < 0) {
    return -1;
  }

  while (!is_stack_empty(&reversed)) {
    Token *cur = NULL;
    pop(&reversed, &cur);

    assert(cur->type == LINE);

    if (add_child_to_token(parent_block, cur) < 0) {
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

char *get_token_type_str(TokenType token_type) {
  switch (token_type) {
  case HEADING:
    return "HEADING";
  case DOCUMENT:
    return "DOCUMENT";
  case TEXT:
    return "TEXT";
  case PARAGRAPH:
    return "PARAGRAPH";
  case LINE:
    return "LINE";
  case BOLD:
    return "BOLD";
  case ITALIC:
    return "ITALIC";
  default:
    return "UNKNOWN";
  }
}

void print_ast(Token *root, int level) {
  if (!root) {
    return;
  }

  for (size_t i = 0; i < level; i++) {
    printf("  ");
  }

  char *type = get_token_type_str(root->type);
  printf("Start %s\n", type);

  if (root->content) {
    for (size_t i = 0; i < level + 1; i++) {
      printf("  ");
    }
    printf("content: \"%s\"\n", root->content);
  }

  for (size_t i = 0; i < root->child_count; i++) {
    print_ast(root->children[i], level + 1);
  }

  for (size_t i = 0; i < level; i++) {
    printf("  ");
  }

  printf("End %s\n", type);
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
