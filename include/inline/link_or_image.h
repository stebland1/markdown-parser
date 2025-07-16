#ifndef LINK_OR_IMAGE_H
#define LINK_OR_IMAGE_H

#include "inline/parser.h"
#include <stddef.h>

int parse_link_or_image(InlineParserContext *ctx);
int is_open_delimiter(void *item, void *userdata);

#endif
