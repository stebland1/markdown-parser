#include "inline.h"
#include <stdlib.h>

InlineElement *create_inline_element(InlineElementType type, void *data) {
  InlineElement *elem = malloc(sizeof(InlineElement));
  if (!elem) {
    return NULL;
  }

  elem->type = type;

  switch (elem->type) {
  case TOKEN:
    elem->token = data;
    break;
  case DELIMITER:
    elem->delimiter = *(Delimiter *)data;
    break;
  default:
    free(elem);
    return NULL;
  }

  return elem;
}

void free_inline_element(InlineElement *elem) {
  if (!elem) {
    return;
  }

  switch (elem->type) {
  case TOKEN:
    free_token(elem->token);
    break;
  case DELIMITER:
    // Nothing to free
    break;
  }

  free(elem);
}
