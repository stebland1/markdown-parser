#include "utils/stack.h"
#include "inline.h"

// Before pushing to the inline stack, we need to append prev and next
int push_to_inline_stack(Stack *inline_stack, InlineElement *element) {
  InlineElement *prev_item = peek_stack_value(inline_stack);
  if (prev_item) {
    element->prev = prev_item;
    prev_item->next = element;
  }

  if (push(inline_stack, &element) < 0) {
    return -1;
  }

  return 0;
}
