#pragma once

#include "token.h"

#define PARSE_LINE(input) create_and_parse_line_token(input)
#define CHILD(token, i) ((token)->children[i])
#define CONTENT(token) ((token)->content)
#define TYPE(token) ((token)->type)
#define NUM_CHILDREN(token) ((token)->child_count)
#define CHILD_CONTENT(token, i) ((token)->children[i]->content)
#define CHILD_TYPE(token, i) ((token)->children[i]->type)

Token *create_and_parse_line_token(const char *input);
