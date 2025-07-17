#ifndef LINK_OR_IMAGE_H
#define LINK_OR_IMAGE_H

#include "inline/parser.h"
#include <stddef.h>

#define SCREEN_READER_CAP 125

int parse_link_or_image(InlineParserContext *ctx);
int is_open_delimiter(void *item, void *userdata);

#endif
