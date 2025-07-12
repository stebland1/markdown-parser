#ifndef INLINE_ELEMENT_H
#define INLINE_ELEMENT_H

#include "inline.h"

InlineElement *create_inline_element(InlineElementType type, void *data);
void free_inline_element(InlineElement *elem);

#endif
