#ifndef LINK_H
#define LINK_H

#include "utils/stack.h"
#include <stddef.h>

char *parse_link(char *c, Stack *inline_stack, char *text_buf,
                 size_t *text_buf_len);
int is_open_link_delimiter(void *item, void *userdata);

#endif
