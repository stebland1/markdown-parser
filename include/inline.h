#ifndef INLINE_H
#define INLINE_H

#include "stack.h"
#include "token.h"
#include <stdint.h>

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

int parse_line(char *line, Stack *inline_stack);
int is_matching_delimiter(Delimiter *delim1, Delimiter *delim2);
int is_escaped(char *c, char *line);
InlineElement *create_inline_element(InlineElementType type, void *data);
void free_inline_element(InlineElement *elem);
InlineElement *find_open_delimiter(Stack *inline_stack, Delimiter *close_delim);
int flush_text_buf(char *buf, size_t *len, Stack *stack);
int push_to_inline_stack(Stack *inline_stack, InlineElement *element);

#endif
