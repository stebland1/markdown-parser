#ifndef INLINE_H
#define INLINE_H

#include "token.h"
#include <stdint.h>

int parse_line(char *line, Token *line_token);
int is_escaped(char *c, char *line);

#define OPEN_SQUARE_BRACKET '['
#define CLOSE_SQUARE_BRACKET ']'
#define ASTERISK '*'
#define UNDERSCORE '_'

#endif
