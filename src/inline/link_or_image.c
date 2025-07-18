#include "inline/link_or_image.h"
#include "inline/element.h"
#include "inline/parser.h"
#include "inline/stack.h"
#include "token.h"
#include "utils/stack.h"
#include "utils/utils.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
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

int collect_image_alt_tag(InlineElement **child_buf, size_t child_buf_len,
                          Token *token) {
  char buf[SCREEN_READER_CAP];
  size_t buf_len = 0;
  buf[0] = '\0';

  for (size_t i = 0; i < child_buf_len; i++) {
    InlineElement *child = child_buf[i];

    if (child->type == TOKEN && child->token->content != NULL) {
      size_t content_len = strlen(child->token->content);
      size_t n = MIN(SCREEN_READER_CAP - buf_len - 1, content_len);
      memcpy(buf + buf_len, child->token->content, n);
      buf_len += n;
    }

    if (buf_len == SCREEN_READER_CAP - 1) {
      break;
    }
  }

  buf[buf_len] = '\0';
  token->meta->image.alt = strdup(buf);
  if (!token->meta->image.alt) {
    return -1;
  }

  return 0;
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
  InlineElement *children_buf[MAX_CHILD_BUF_SIZE];
  if (pop_until_delimiter(children_buf, &buf_len, ctx->inline_stack,
                          matching_delimiter) < 0) {
    return -1;
  }
  free_inline_element(matching_delimiter);

  if (is_img) {
    if (collect_image_alt_tag(children_buf, buf_len, token) < 0) {
      return -1;
    }
  } else {
    for (size_t i = 0; i < buf_len; i++) {
      if (add_child_to_token(token, children_buf[i]->token) < 0) {
        free_token(token);
        return -1;
      }
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
