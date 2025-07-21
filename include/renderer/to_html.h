#ifndef TO_HTML
#define TO_HTML

#include "token.h"

typedef struct Attribute {
  char key[6];
  char *value;
} Attribute;

typedef struct HtmlParserUserOptions {
  int pretty;
  int omit_body;
} HtmlParserUserOptions;

typedef struct HtmlParserOptions {
  HtmlParserUserOptions user;
  int is_last;
  int is_first;
  int has_nested_list;
  int level;
} HtmlParserOptions;

int is_self_closing(TokenType type);
char *get_tag_from_type(TokenType type, void *meta);
void free_attributes(Attribute *attributes, size_t num_attributes);
void to_html(Token *root, HtmlParserOptions *opts);
void render_as_html(Token *root, HtmlParserUserOptions user_opts);
int get_attributes(Token *token, Attribute *attributes);
void print_indent(int level);
int is_block_element(TokenType type);
void render_children(Token *root, HtmlParserOptions *opts);

#endif
