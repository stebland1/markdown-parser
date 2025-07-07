#ifndef STACK_H
#define STACK_H

#include <stddef.h>

typedef struct {
  void *items;
  size_t count;
  size_t capacity;
  size_t item_size;
} Stack;

int create_stack(Stack *stack, size_t item_size);
int push(Stack *stack, void *item);
int pop(Stack *stack, void *out);
void *peek_stack(Stack *stack);
void free_stack(Stack *stack);
int is_stack_empty(Stack *stack);

#endif
