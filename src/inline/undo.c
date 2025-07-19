#include "inline/undo.h"
#include "inline/element.h"
#include "inline/emphasis.h"
#include "utils/utils.h"
#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

char *token_to_str_wrapper(void *ptr) {
  return token_to_markdown((Token *)ptr);
}

char *token_to_markdown(Token *token) {
  switch (token->type) {
  case TEXT:
    return strdup(token->content);
  case LINK: {
    char *child_str = join_items_as_str(
        (void **)token->children, token->child_count, token_to_str_wrapper);
    if (!child_str) {
      return NULL;
    }
    return concat(5, "[", child_str, "](", token->meta->link.href, ")");
  }
  case IMAGE:
    return concat(5, "![", token->meta->image.alt, "](", token->meta->image.src,
                  ")");
  case BOLD:
  case ITALIC: {
    char *child_str = join_items_as_str(
        (void **)token->children, token->child_count, token_to_str_wrapper);
    if (!child_str) {
      return NULL;
    }
    char emphasis[MAX_DELIMITER_LEN];
    emphasis_token_to_str(&token->meta->emphasis, emphasis, MAX_DELIMITER_LEN);
    return concat(3, emphasis, child_str, emphasis);
  }
  default:
    return strdup("");
  }
}

char *delimiter_to_markdown(Delimiter *delimiter) {
  assert(delimiter->count <= MAX_DELIMITER_LEN);
  char *out = malloc(MAX_DELIMITER_LEN + 1);
  if (!out) {
    return NULL;
  }
  memset(out, delimiter->symbol, delimiter->count);
  out[delimiter->count] = '\0';
  return out;
}

char *join_items_as_str(void **items, size_t count, ToStringFn to_string) {
  size_t total_len = 0;
  char **parts = malloc(count * sizeof(char *));
  if (!parts) {
    return NULL;
  }

  for (size_t i = 0; i < count; i++) {
    char *child_text = to_string(items[i]);
    if (!child_text) {
      for (size_t j = 0; j < i; j++) {
        free(parts[j]);
      }
      free(parts);
      return NULL;
    }

    parts[i] = child_text;
    total_len += strlen(child_text);
  }

  char *out = malloc(total_len + 1 /* null */);
  if (!out) {
    for (size_t i = 0; i < count; i++) {
      free(parts[i]);
    }
    free(parts);
    return NULL;
  }
  size_t outlen = 0;

  for (size_t i = 0; i < count; i++) {
    size_t len = strlen(parts[i]);
    memcpy(out + outlen, parts[i], len);
    outlen += len;
    free(parts[i]);
  }

  out[outlen] = '\0';
  return out;
}
