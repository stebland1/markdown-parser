#ifndef LIST_H
#define LIST_H

#include "context.h"
#include "token.h"

#define LIST_GROWTH_FACTOR 4

char *parse_list_item(char *c, ListData *meta);
Token *create_list_token(ParserContext *ctx, char *content, ListData *meta);
int add_parent_list_only(ParserContext *ctx, Token *token, void *_);
int handle_list_item(ParserContext *ctx, char *line);
int stop_when_indentation_is_equal(Token *token, void *userdata);
int is_nested_list(ListData *current, Token *active);
Token *push_new_list_token(ParserContext *ctx, char *list_item, ListData *data);
char *handle_unordered_list_item(char *c);
char *handle_ordered_list_item(char *c, ListData *meta);
int is_unordered_list_item(char c);

#endif
