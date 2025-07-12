#ifndef INLINE_STACK_H
#define INLINE_STACK_H

#include "inline.h"
#include "utils/stack.h"

int push_to_inline_stack(Stack *inline_stack, InlineElement *element);
int flush_text_into_stack(char *buf, size_t *len, Stack *inline_stack);

#endif
