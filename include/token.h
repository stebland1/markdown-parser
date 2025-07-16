#ifndef TOKEN_H
#define TOKEN_H

#include <stddef.h>

typedef enum {
  DOCUMENT,
  HEADING,
  PARAGRAPH,
  LINE,
  TEXT,
  BOLD,
  LINK,
  LIST,
  ORDERED_LIST,
  LIST_ITEM,
  IMAGE,
  ITALIC,
  UNKNOWN,
} TokenType;

typedef struct Token Token;

typedef struct HeadingData {
  unsigned int level;
} HeadingData;

typedef struct ListData {
  char symbol;
  int indentation;
  int parent;
  Token *last;
} ListData;

typedef struct LinkData {
  char *href;
} LinkData;

typedef struct ImageData {
  char *src;
} ImageData;

typedef union TokenMeta {
  HeadingData heading;
  ListData list;
  LinkData link;
  ImageData image;
} TokenMeta;

typedef struct Token {
  TokenType type;
  char *content;
  TokenMeta *meta;
  struct Token **children;
  size_t child_count;
  size_t child_capacity;
} Token;

Token *create_token(TokenType type, size_t child_increment, char *content,
                    void *meta);
void free_token(Token *token);
int add_child_to_token(Token *token, Token *child);

#endif
