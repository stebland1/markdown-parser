#ifndef INLINE_STACK_H
#define INLINE_STACK_H

#include "inline/element.h"
#include "utils/stack.h"

#define MAX_CHLD_BUF_SIZE 64

int push_to_inline_stack(Stack *inline_stack, InlineElement *element);
int flush_text_into_stack(char *buf, size_t *len, Stack *inline_stack);
int merge_unmatched_delimiters(InlineElement *delimiter, Stack *inline_stack);
int pop_until_delimiter(InlineElement **buf, size_t *buf_len, Stack *stack,
                        InlineElement *delimiter);
#endif
