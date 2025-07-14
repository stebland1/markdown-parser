#include "inline/element.h"
#include "inline/parser.h"
#include "inline/stack.h"
#include "token.h"
#include "utils/stack.h"
#include <stddef.h>

int is_open_link_delimiter(void *item, void *userdata) {
  InlineElement *element = (InlineElement *)item;
  return element->type == DELIMITER &&
         element->delimiter.symbol == OPEN_SQUARE_BRACKET;
}

char *parse_link(char *c, Stack *inline_stack, char *text_buf,
                 size_t *text_buf_len) {
  InlineElement *matching_delimiter =
      find_stack(inline_stack, is_open_link_delimiter, NULL);
  if (!matching_delimiter) {
    text_buf[(*text_buf_len)++] = *c++;
    return c;
  }

  if (*(c + 1) == '\0' || *(c + 1) != '(') {
    text_buf[(*text_buf_len)++] = *c++;
    return c;
  }

  if (flush_text_into_stack(text_buf, text_buf_len, inline_stack) < 0) {
    return NULL;
  }

  c += 2;
  while (*c && *c != ')') {
    text_buf[(*text_buf_len)++] = *c;
    c++;
  }

  if (*c == '\0') {
    return c;
  }

  c++;
  text_buf[(*text_buf_len)] = '\0';
  *text_buf_len = 0;

  Token *link_token = create_token(LINK, 1, text_buf, NULL);
  if (!link_token) {
    return NULL;
  }

  size_t buf_len = 0;
  InlineElement *children_buf[MAX_CHLD_BUF_SIZE];
  if (pop_until_delimiter(children_buf, &buf_len, inline_stack,
                          matching_delimiter) < 0) {
    return NULL;
  }
  free_inline_element(matching_delimiter);

  for (size_t i = 0; i < buf_len; i++) {
    if (add_child_to_token(link_token, children_buf[i]->token) < 0) {
      free_token(link_token);
      return NULL;
    }
  }

  InlineElement *link_element = create_inline_element(TOKEN, link_token);
  if (!link_element) {
    free_token(link_token);
    return NULL;
  }
  if (push_to_inline_stack(inline_stack, link_element) < 0) {
    free_token(link_token);
    return NULL;
  }
  return c;
}
