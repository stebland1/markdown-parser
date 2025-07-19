#ifndef UNDO
#define UNDO

#include "inline/element.h"
#include "token.h"
#include <stddef.h>

typedef char *(*ToStringFn)(void *item);

char *token_to_markdown(Token *token);
char *join_items_as_str(void **items, size_t count, ToStringFn to_string);
char *element_to_markdown(void *item);
char *delimiter_to_markdown(Delimiter *delimiter);

#endif
