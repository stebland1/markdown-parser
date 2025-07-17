#include "inline/parser.h"
#include "inline/element.h"
#include "inline/emphasis.h"
#include "inline/link_or_image.h"
#include "inline/stack.h"
#include "token.h"
#include "utils/stack.h"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>

int is_escaped(char *c, char *line) {
  if (c <= line) {
    return 0;
  }

  return *(c - 1) == '\\';
}

int is_image(InlineParserContext *ctx) {
  return ctx->c > ctx->line && *(ctx->c - 1) == EXCLAMATION_MARK;
}

int parse_line(char *line, Token *line_token) {
  Stack inline_stack;
  if (create_stack(&inline_stack, sizeof(InlineElement *)) < 0) {
    return -1;
  }

  InlineParserContext ctx = {
      .inline_stack = &inline_stack,
      .c = line,
      .line = line,
      .text_buf_len = 0,
  };

  while (*ctx.c) {
    switch (*ctx.c) {
    case EXCLAMATION_MARK:
      if (*(ctx.c + 1) == OPEN_SQUARE_BRACKET) {
        ctx.c++;
      } else {
        ctx.text_buf[ctx.text_buf_len++] = *ctx.c++;
      }
      break;
    case OPEN_SQUARE_BRACKET: {
      if (flush_text_into_stack(&ctx) < 0) {
        return -1;
      }

      Delimiter delimiter = {.symbol = OPEN_SQUARE_BRACKET,
                             .count = 1,
                             .prefix = is_image(&ctx) ? *(ctx.c - 1) : 0};
      InlineElement *open_link_delimiter_element =
          create_inline_element(DELIMITER, &delimiter);

      if (push_to_inline_stack(&inline_stack, open_link_delimiter_element) <
          0) {
        return -1;
      }
      ctx.c++;
      break;
    };
    case CLOSE_SQUARE_BRACKET: {
      if (parse_link_or_image(&ctx) < 0) {
        return -1;
      }
      break;
    }
    case ASTERISK:
    case UNDERSCORE: {
      if (handle_emphasis(&ctx) < 0) {
        return -1;
      }
      break;
    }
    default:
      ctx.text_buf[ctx.text_buf_len++] = *ctx.c++;
      break;
    }
  }

  if (flush_text_into_stack(&ctx) < 0) {
    return -1;
  }

  for (size_t i = 0; i < inline_stack.count; i++) {
    InlineElement *inline_element = ((InlineElement **)inline_stack.items)[i];
    if (inline_element->type == DELIMITER) {
      if (merge_unmatched_delimiters(inline_element, &inline_stack) < 0) {
        return -1;
      }
      continue;
    }

    if (add_child_to_token(line_token, inline_element->token) < 0) {
      return -1;
    }
  }

  return 0;
}
