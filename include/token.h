#ifndef TOKEN_H
#define TOKEN_H

#include <stddef.h>

typedef enum {
  DOCUMENT,
  HEADING,
  PARAGRAPH,
  TEXT,
} TokenType;

typedef struct Token {
  TokenType type;
  char *content;
  unsigned int level;
  struct Token **children;
  size_t child_count;
  size_t child_capacity;
} Token;

Token *create_token(TokenType type, size_t child_increment, char *content);
void free_token(Token *token);
int add_child_to_token(Token *token, Token *child);

#endif
