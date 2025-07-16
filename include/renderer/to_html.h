#ifndef TO_HTML
#define TO_HTML

#include "token.h"

typedef struct Attribute {
  char key[6];
  char *value;
} Attribute;

int is_self_closing(TokenType type);
char *get_tag_from_type(TokenType type, void *meta);
void free_attributes(Attribute *attributes, size_t num_attributes);
void to_html(Token *root, Token *prev);
int get_attributes(Token *token, Attribute *attributes);

#endif
