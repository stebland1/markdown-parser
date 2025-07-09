#ifndef INLINE_H
#define INLINE_H

#include "stack.h"
#include "token.h"
#include <stdint.h>

typedef struct {
  char symbol;   // the actual delimiter i.e. *.
  uint8_t count; // the number of them i.e. 2.
  uint8_t pos;
  uint8_t can_open;
  uint8_t can_close;
} Delimiter;

typedef struct {
  enum { TOKEN, DELIMITER } type;
  union {
    Token *token;
    Delimiter delimiter;
  };
} InlineElement;

int parse_line(char *line, Stack *inline_stack);
int handle_emphasis(char *line, Stack *inline_stack);
int is_matching_delimiter(Delimiter *delim1, Delimiter *delim2);
int can_open_emphasis(char *c, char *line);
int can_close_emphasis(char *c, char *line);
int is_escaped(char *c, char *line);

#endif
