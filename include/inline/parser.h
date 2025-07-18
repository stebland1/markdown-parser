#ifndef INLINE_H
#define INLINE_H

#include "token.h"
#include "utils/stack.h"
#include <stddef.h>
#include <stdint.h>

#define OPEN_SQUARE_BRACKET '['
#define CLOSE_SQUARE_BRACKET ']'
#define ASTERISK '*'
#define UNDERSCORE '_'
#define EXCLAMATION_MARK '!'
#define BACKTICK '`'

typedef struct InlineParserContext {
  Stack *inline_stack;
  char *c;
  char *line;
  char text_buf[1024];
  size_t text_buf_len;
} InlineParserContext;

int parse_line(char *line, Token *line_token);
int is_escaped(char *c, char *line);

#endif
