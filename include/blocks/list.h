#ifndef LIST_H
#define LIST_H

#include "parser.h"
#include "token.h"

#define LIST_GROWTH_FACTOR 4

int list_block_start(ListData *data, ParserContext *ctx);
int add_list_item_to_list(char *line, ParserContext *ctx);
char *get_list_item(char *c, ListData *data);

#endif
