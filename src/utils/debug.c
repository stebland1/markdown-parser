#include "inline/element.h"
#include "token.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>

char *get_inline_elem_type(InlineElementType type) {
  switch (type) {
  case TOKEN:
    return "TOKEN";
  case DELIMITER:
    return "DELIMITER";
  default:
    return "UNKNOWN";
  }
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
  case LINK:
    return "LINK";
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

  if (root->type == HEADING && root->meta != NULL) {
    for (size_t i = 0; i < level + 1; i++) {
      printf("  ");
    }
    printf("level: \"%d\"\n", root->meta->heading.level);
  }

  for (size_t i = 0; i < root->child_count; i++) {
    print_ast(root->children[i], level + 1);
  }

  for (size_t i = 0; i < level; i++) {
    printf("  ");
  }

  printf("End %s\n", type);
}
