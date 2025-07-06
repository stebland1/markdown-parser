#include "utils.h"
#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STACK_BUF_INCREMENT 8

void trim(char *str) {
  char *start = str;
  for (; isspace(*start); start++)
    ;

  if (*start == '\0') {
    *str = '\0';
    return;
  }

  char *end = str + strlen(str) - 1;
  for (; end > start && isspace(*end); end--)
    ;
  *(end + 1) = '\0';

  if (start > str) {
    memmove(str, start, end - start + 2);
  }
}

char *escape_json_str(char *input) {
  size_t len = strlen(input);
  char *out = malloc(len * 6 + 1);

  if (!out)
    return NULL;

  char *p = out;

  for (size_t i = 0; i < len; i++) {
    unsigned char c = input[i];

    switch (c) {
    case '\"':
      *p++ = '\\';
      *p++ = '\"';
      break;
    case '\\':
      *p++ = '\\';
      *p++ = '\\';
      break;
    case '\n':
      *p++ = '\\';
      *p++ = 'n';
      break;
    case '\t':
      *p++ = '\\';
      *p++ = 't';
      break;
    case '\b':
      *p++ = '\\';
      *p++ = 'b';
      break;
    case '\f':
      *p++ = '\\';
      *p++ = 'f';
      break;
    case '\r':
      *p++ = '\\';
      *p++ = 'r';
      break;
    default:
      if (c < 0x20) {
        p += sprintf(p, "\\u%04x", c);
      } else {
        *p++ = c;
      }
      break;
    }
  }

  *p = '\0';
  return out;
}

int create_stack(Stack *stack, size_t item_size) {
  stack->item_size = item_size;
  stack->items = malloc(stack->item_size * STACK_BUF_INCREMENT);
  if (!stack->items) {
    return -1;
  }
  stack->capacity = STACK_BUF_INCREMENT;
  stack->count = 0;
  return 0;
}

int push(Stack *stack, void *item) {
  if (stack->count == stack->capacity) {
    void *new_items = realloc(
        stack->items, (stack->count + STACK_BUF_INCREMENT) * stack->item_size);
    if (!new_items) {
      return -1;
    }
    stack->items = new_items;
    stack->capacity += STACK_BUF_INCREMENT;
  }

  void *target = (char *)stack->items + (stack->count * stack->item_size);
  memcpy(target, item, stack->item_size);
  stack->count++;
  return 0;
}

int pop(Stack *stack, void *out) {
  if (stack->count == 0) {
    return -1;
  }

  void *target = (char *)stack->items + (--stack->count * stack->item_size);
  memcpy(out, target, stack->item_size);
  return 0;
}

void *peek_stack(Stack *stack) {
  if (stack->count == 0) {
    return NULL;
  }

  return (char *)stack->items + ((stack->count - 1) * stack->item_size);
}

void free_stack(Stack *stack) {
  if (stack->items) {
    free(stack->items);
    stack->items = NULL;
  }

  stack->count = 0;
  stack->capacity = 0;
}

int is_blank_line(char *line) {
  while (isspace(*line))
    line++;
  return *line == '\0';
}
