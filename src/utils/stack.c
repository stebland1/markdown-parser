#include "utils/stack.h"
#include <stdlib.h>
#include <string.h>

#define STACK_BUF_INCREMENT 8

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

int is_stack_empty(Stack *stack) { return stack->count == 0; }

int reverse_stack(Stack *dst, Stack *src) {
  if (create_stack(dst, src->item_size) < 0) {
    return -1;
  }

  while (!is_stack_empty(src)) {
    void *tok = NULL;
    pop(src, &tok);
    push(dst, &tok);
  }

  return 0;
}
