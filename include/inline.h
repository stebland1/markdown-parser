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
int is_matching_delimiter(Delimiter *delim1, Delimiter *delim2);
int is_escaped(char *c, char *line);
InlineElement *find_open_delimiter(Stack *inline_stack, Delimiter *close_delim);
int flush_text_buf(char *buf, size_t *len, Stack *inline_stack);

#endif
