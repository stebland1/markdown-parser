#ifndef EMPHASIS_H
#define EMPHASIS_H

#include "inline.h"
#include "token.h"
#include "utils/stack.h"

TokenType get_emphasis_token_type(char symbol, int count);
int create_emphasis_token(TokenType token_type, InlineElement **children,
                          size_t children_len, Stack *inline_stack);
char *handle_emphasis(char *c, char *line, char *text_buf, size_t *text_buf_len,
                      Stack *inline_stack);
int can_open_emphasis(char *c, char *line);
int can_close_emphasis(char *c, char *line);
int is_matching_inline_delim(void *item, void *userdata);

#endif
