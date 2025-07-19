#ifndef STACK_H
#define STACK_H

#include <stddef.h>

typedef struct {
  void *items;
  size_t count;
  size_t capacity;
  size_t item_size;
} Stack;

typedef int (*StackPredicate)(void *item, void *userdata);

int create_stack(Stack *stack, size_t item_size);
int push(Stack *stack, void *item);
int pop(Stack *stack, void *out);
void *peek_stack_raw(Stack *stack);
void *peek_stack_value(Stack *stack);
void free_stack(Stack *stack);
int is_stack_empty(Stack *stack);
void *find_stack(Stack *stack, StackPredicate predicate, void *userdata);
void remove_from_stack(Stack *stack, void *item);

#endif
