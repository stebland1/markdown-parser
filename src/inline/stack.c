#include "utils/stack.h"
#include "inline/element.h"
#include "inline/parser.h"
#include "inline/stack.h"
#include "utils/utils.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

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

int concat_with_prev_token(char *line, InlineElement *prev) {
  char *new_content = concat(2, prev->token->content, line);
  if (!new_content) {
    return -1;
  }

  free(prev->token->content);
  prev->token->content = new_content;
  return 0;
}

int flush_text_into_stack(InlineParserContext *ctx) {
  if (ctx->text_buf_len == 0) {
    return 0;
  }

  ctx->text_buf[ctx->text_buf_len] = '\0';
  ctx->text_buf_len = 0;

  InlineElement *prev = peek_stack_value(ctx->inline_stack);

  // Try to concatenate with the prev text token (if there is one).
  if (prev && prev->type == TOKEN && prev->token->type == TEXT) {
    if (concat_with_prev_token(ctx->text_buf, prev) < 0) {
      return -1;
    }

    return 0;
  }

  Token *token = create_token(TEXT, 0, ctx->text_buf, NULL);
  if (!token) {
    return -1;
  }

  InlineElement *elem = create_inline_element(TOKEN, token);
  if (!elem) {
    free_token(token);
    return -1;
  }

  if (push_to_inline_stack(ctx->inline_stack, elem) < 0) {
    free_inline_element(elem);
    return -1;
  }

  return 0;
}

int merge_unmatched_delimiters(InlineElement *delimiter, Stack *inline_stack) {
  InlineElement *prev = delimiter->prev;
  InlineElement *next = delimiter->next;

  char buf[MAX_DELIMITER_LEN];
  memset(buf, delimiter->delimiter.symbol, delimiter->delimiter.count);

  if (prev && next) {
    char *new_content =
        concat(3, prev->token->content, buf, next->token->content);
    if (!new_content) {
      return -1;
    }
    free(prev->token->content);
    prev->token->content = new_content;

    // pop the `next` item as it's now been concatenated.
    free_inline_element(delimiter);
    InlineElement *next_in_stack = NULL;
    pop(inline_stack, &next_in_stack);
    free_inline_element(next_in_stack);
    return 0;
  }

  if (prev) {
    char *new_content = concat(2, prev->token->content, buf);
    free_inline_element(delimiter);
    if (!new_content) {
      return -1;
    }
    free(prev->token->content);
    prev->token->content = new_content;
    return 0;
  }

  if (next) {
    char *new_content = concat(2, buf, next->token->content);
    free_inline_element(delimiter);
    if (!new_content) {
      return -1;
    }
    free(next->token->content);
    next->token->content = new_content;
    return 0;
  }

  return 0;
}

int pop_until_delimiter(InlineElement **buf, size_t *buf_len, Stack *stack,
                        InlineElement *delimiter) {
  InlineElement *cur = NULL;

  do {
    if (pop(stack, &cur) < 0) {
      for (size_t i = 0; i < *buf_len; i++) {
        free_inline_element(buf[i]);
      }
      return -1;
    }

    if (cur != delimiter) {
      if (*buf_len >= MAX_CHLD_BUF_SIZE) {
        free_inline_element(cur);
        return -1;
      }
      buf[(*buf_len)++] = cur;
    }
  } while (cur != delimiter);

  reverse_list(buf, *buf_len, sizeof(InlineElement *));

  return 0;
}
