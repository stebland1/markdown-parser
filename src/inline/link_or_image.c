#include "inline/element.h"
#include "inline/parser.h"
#include "inline/stack.h"
#include "token.h"
#include "utils/stack.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>

int is_open_delimiter(void *item, void *userdata) {
  InlineElement *element = (InlineElement *)item;
  return element->type == DELIMITER &&
         element->delimiter.symbol == OPEN_SQUARE_BRACKET;
}

Token *create_link_token(InlineParserContext *ctx) {
  LinkData meta = {.href = strdup(ctx->text_buf)};
  Token *link_token = create_token(LINK, 1, NULL, &meta);
  if (!link_token) {
    return NULL;
  }

  return link_token;
}

Token *create_image_token(InlineParserContext *ctx) {
  ImageData meta = {.src = strdup(ctx->text_buf)};
  Token *image_token = create_token(IMAGE, 1, NULL, &meta);
  if (!image_token) {
    return NULL;
  }

  return image_token;
}

int parse_link_or_image(InlineParserContext *ctx) {
  InlineElement *matching_delimiter =
      find_stack(ctx->inline_stack, is_open_delimiter, NULL);
  if (!matching_delimiter) {
    ctx->text_buf[ctx->text_buf_len++] = *ctx->c++;
    return 0;
  }

  if (*(ctx->c + 1) == '\0' || *(ctx->c + 1) != '(') {
    ctx->text_buf[ctx->text_buf_len++] = *ctx->c++;
    return 0;
  }

  if (flush_text_into_stack(ctx) < 0) {
    return -1;
  }

  ctx->c += 2;
  while (*ctx->c && *ctx->c != ')') {
    ctx->text_buf[ctx->text_buf_len++] = *ctx->c;
    ctx->c++;
  }

  if (*ctx->c == '\0') {
    return 0;
  }

  ctx->c++;
  ctx->text_buf[ctx->text_buf_len] = '\0';
  ctx->text_buf_len = 0;

  int is_img = matching_delimiter->delimiter.prefix == EXCLAMATION_MARK;
  Token *token = is_img ? create_image_token(ctx) : create_link_token(ctx);
  if (!token) {
    return -1;
  }

  size_t buf_len = 0;
  InlineElement *children_buf[MAX_CHLD_BUF_SIZE];
  if (pop_until_delimiter(children_buf, &buf_len, ctx->inline_stack,
                          matching_delimiter) < 0) {
    return -1;
  }
  free_inline_element(matching_delimiter);

  for (size_t i = 0; i < buf_len; i++) {
    if (add_child_to_token(token, children_buf[i]->token) < 0) {
      free_token(token);
      return -1;
    }
  }

  InlineElement *link_or_image_elem = create_inline_element(TOKEN, token);
  if (!link_or_image_elem) {
    free_token(token);
    return -1;
  }

  if (push_to_inline_stack(ctx->inline_stack, link_or_image_elem) < 0) {
    free_token(token);
    return -1;
  }

  return 0;
}
