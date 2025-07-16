#ifndef INLINE_ELEMENT_H
#define INLINE_ELEMENT_H

#include "token.h"
#define MAX_DELIMITER_LEN 6

typedef struct {
  char symbol; // the actual delimiter i.e. *.
  int count;   // the number of them i.e. 2.
  char prefix;
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

InlineElement *create_inline_element(InlineElementType type, void *data);
void free_inline_element(InlineElement *elem);

#endif
