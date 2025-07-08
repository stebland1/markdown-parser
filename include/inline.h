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

#endif
