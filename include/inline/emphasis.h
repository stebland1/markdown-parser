#ifndef EMPHASIS_H
#define EMPHASIS_H

#include "inline/element.h"
#include "inline/parser.h"
#include "token.h"
#include "utils/stack.h"

TokenType get_emphasis_token_type(char symbol, int count);
int create_emphasis_token(TokenType token_type, InlineElement **children,
                          size_t children_len, Stack *inline_stack,
                          EmphasisData *meta);
char *handle_emphasis(InlineParserContext *ctx);
int is_matching_inline_delim(void *item, void *userdata);
int is_punctuation(char c);
int is_whitespace(char c);
void classify_delimiter_runs(char *start, char *end, char *line, int *can_open,
                             int *can_close);

#endif
