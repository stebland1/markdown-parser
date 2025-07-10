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
      if (parse_line(line, inline_stack) < 0) {
        return -1;
      }
      break;
    default: {
      if (handle_paragraph(block_stack) < 0) {
        return -1;
      }
      if (parse_line(line, inline_stack) < 0) {
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

int handle_text(char *line, Stack *inline_stack) {
  InlineElement **stack_top_ptr = peek_stack(inline_stack);
  InlineElement *stack_top = stack_top_ptr ? *stack_top_ptr : NULL;

  if (stack_top && stack_top->type == TOKEN && stack_top->token->type == TEXT) {
    size_t old_len =
        stack_top->token->content ? strlen(stack_top->token->content) : 0;
    size_t new_len = old_len + 1 + strlen(line) + 1;

    char *new_content = realloc(stack_top->token->content, new_len);
    if (!new_content) {
      return -1;
    }

    stack_top->token->content = new_content;
    stack_top->token->content[old_len] = ' ';
    strcpy(stack_top->token->content + old_len + 1, line);
    return 0;
  }

  Token *token = create_token(TEXT, 0, line);
  if (!token) {
    return -1;
  }

  InlineElement *elem = create_inline_element(TOKEN, token);
  if (!elem) {
    free_token(token);
    return -1;
  }

  if (push_to_inline_stack(inline_stack, elem) < 0) {
    free_inline_element(elem);
    return -1;
  }

  return 0;
}

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

int handle_unmatched_delimiter(InlineElement *delimiter, Stack *inline_stack) {
  InlineElement *prev = delimiter->prev;
  InlineElement *next = delimiter->next;

  char buf[MAX_DELIMITER_LEN];
  memset(buf, delimiter->delimiter.symbol, delimiter->delimiter.count);

  if (prev && next) {
    char *new_content =
        concat(3, prev->token->content, buf, next->token->content);
    if (!new_content) {
      return -1;
    }
    free(prev->token->content);
    prev->token->content = new_content;

    // pop the `next` item as it's now been concatenated.
    free_inline_element(delimiter);
    InlineElement *next_in_stack = NULL;
    pop(inline_stack, &next_in_stack);
    free_inline_element(next_in_stack);
    return 0;
  }

  if (prev) {
    char *new_content = concat(2, prev->token->content, buf);
    free_inline_element(delimiter);
    if (!new_content) {
      return -1;
    }
    free(prev->token->content);
    prev->token->content = new_content;
    return 0;
  }

  if (next) {
    char *new_content = concat(2, buf, next->token->content);
    free_inline_element(delimiter);
    if (!new_content) {
      return -1;
    }
    free(next->token->content);
    next->token->content = new_content;
    return 0;
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
    InlineElement *cur = NULL;
    pop(&reversed, &cur);

    assert(cur != NULL);

    if (cur->type == DELIMITER) {
      if (handle_unmatched_delimiter(cur, &reversed) < 0) {
        return -1;
      }
      continue;
    }

    if (add_child_to_token(parent_block, cur->token) < 0) {
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
    printf("\t");
  }

  char *type = get_token_type_str(root->type);
  printf("- type: %s\n", type);

  for (size_t i = 0; i < level; i++) {
    printf("\t");
  }

  if (root->content) {
    printf("- content: \"%s\"\n", root->content);
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
