#include "token.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Token *create_token(TokenType type, size_t child_capacity, char *content) {
  Token *token = malloc(sizeof(Token));
  if (!token) {
    return NULL;
  }

  token->type = type;
  token->content = NULL;
  token->children = NULL;
  token->child_count = 0;
  token->child_capacity = 0;
  token->level = 0;

  if (child_capacity > 0) {
    token->children = malloc(sizeof(Token *) * child_capacity);
    if (!token->children) {
      free(token);
      return NULL;
    }
    token->child_capacity = child_capacity;
  }

  if (content) {
    token->content = strdup(content);
    if (!token->content) {
      free(token->children);
      free(token);
    }
  }

  return token;
}

void free_token(Token *token) {
  if (!token)
    return;

  for (size_t i = 0; i < token->child_count; ++i) {
    free_token(token->children[i]);
  }

  free(token->children);
  free(token->content);
  free(token);
}

int add_child_to_token(Token *token, Token *child) {
  if (token->child_count == token->child_capacity) {
    Token **new_items =
        realloc(token->children, sizeof(Token *) * (token->child_capacity * 2));
    if (!new_items) {
      return -1;
    }
    token->child_capacity *= 2;
    token->children = new_items;
  }

  token->children[token->child_count++] = child;
  return 0;
}
