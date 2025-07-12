#ifndef INLINE_H
#define INLINE_H

#include "token.h"
#include "utils/stack.h"
#include <stdint.h>

#define MAX_DELIMITER_LEN 6

typedef struct {
  char symbol; // the actual delimiter i.e. *.
  int count;   // the number of them i.e. 2.
} Delimiter;

typedef enum { TOKEN, DELIMITER } InlineElementType;

typedef struct InlineElement {
  InlineElementType type;
  union {
    Token *token;
    Delimiter delimiter;
  };
  struct InlineElement *prev;
  struct InlineElement *next;
} InlineElement;

int parse_line(char *line, Token *line_token);
int is_escaped(char *c, char *line);
int flush_text_buf(char *buf, size_t *len, Stack *inline_stack);

#endif
