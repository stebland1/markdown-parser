#ifndef LINK_H
#define LINK_H

#include "inline/parser.h"
#include <stddef.h>

int parse_link(InlineParserContext *ctx);
int is_open_link_delimiter(void *item, void *userdata);

#endif
