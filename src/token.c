#include "token.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Token *create_token(TokenType type, size_t child_capacity, char *content,
                    void *meta) {
  Token *token = malloc(sizeof(Token));
  if (!token) {
    return NULL;
  }

  token->type = type;
  token->content = NULL;
  token->children = NULL;
  token->child_count = 0;
  token->child_capacity = 0;
  token->meta = NULL;

  if (meta) {
    token->meta = malloc(sizeof(TokenMeta));
    if (!token->meta) {
      free_token(token);
      return NULL;
    }

    switch (type) {
    case HEADING:
      token->meta->heading = *(HeadingData *)meta;
      break;
    default:
      free(token->meta);
      break;
    }
  }

  if (child_capacity > 0) {
    token->children = malloc(sizeof(Token *) * child_capacity);
    if (!token->children) {
      free_token(token);
      return NULL;
    }
    token->child_capacity = child_capacity;
  }

  if (content) {
    token->content = strdup(content);
    if (!token->content) {
      free_token(token);
      return NULL;
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

  free(token->meta);
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
