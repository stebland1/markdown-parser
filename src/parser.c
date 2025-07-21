#include "parser.h"
#include "blocks/block_quote.h"
#include "blocks/code_block.h"
#include "blocks/heading.h"
#include "blocks/list.h"
#include "blocks/paragraph.h"
#include "blocks/stack.h"
#include "blocks/thematic_break.h"
#include "context.h"
#include "token.h"
#include "utils/debug.h"
#include <assert.h>
#include <ctype.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define FRONT_MATTER_DELIM "---"

typedef enum {
  LINE_TYPE_FRONT_MATTER,
  LINE_TYPE_BLANK,
  LINE_TYPE_HEADING,
  LINE_TYPE_PARAGRAPH,
  LINE_TYPE_LIST,
  LINE_TYPE_BLOCK_QUOTE,
  LINE_TYPE_CODE_BLOCK,
  LINE_TYPE_CODE_BLOCK_CLOSE,
  LINE_TYPE_CODE_BLOCK_OPEN,
  LINE_TYPE_THEMATIC_BREAK,
} LineType;

int is_thematic_break(char *p, ParserContext *ctx) {
  char c = *p;
  if (c != '-' && c != '_' && c != '*') {
    return 0;
  }

  int count = 0;
  while (*p == c) {
    count++;
    p++;
  }

  return *p == '\0' && count >= 3;
}

int classify_line_type(char *line, ParserContext *ctx) {
  char *p = line;
  int indentation = 0;
  while (isspace(*line)) {
    indentation++;
    line++;
  }

  if (ctx->front_matter_state == FRONT_MATTER_NONE &&
      strncmp(line, FRONT_MATTER_DELIM, 3) == 0) {
    ctx->front_matter_state = FRONT_MATTER_IN;
    return LINE_TYPE_FRONT_MATTER;
  }

  if (ctx->front_matter_state == FRONT_MATTER_IN) {
    if (strncmp(line, FRONT_MATTER_DELIM, 3) == 0) {
      ctx->front_matter_state = FRONT_MATTER_DONE;
    }
    return LINE_TYPE_FRONT_MATTER;
  }

  if (indentation <= 3 && is_thematic_break(line, ctx)) {
    return LINE_TYPE_THEMATIC_BREAK;
  }

  if (ctx->code_block.parsing && strncmp(line, "```", 3) == 0) {
    ctx->code_block.parsing = 0;
    return LINE_TYPE_CODE_BLOCK_CLOSE;
  }

  if (ctx->code_block.parsing) {
    return LINE_TYPE_CODE_BLOCK;
  }

  if (*line == '#') {
    return LINE_TYPE_HEADING;
  }

  if (*line == '>') {
    return LINE_TYPE_BLOCK_QUOTE;
  }

  if (*p == '\0') {
    return LINE_TYPE_BLANK;
  }

  if (!ctx->code_block.parsing && strncmp(p, "```", 3) == 0) {
    ctx->code_block.parsing = 1;
    return LINE_TYPE_CODE_BLOCK_OPEN;
  }

  if (parse_list_item(line, NULL)) {
    return LINE_TYPE_LIST;
  }

  return LINE_TYPE_PARAGRAPH;
};

int parse_file(FILE *file, ParserContext *ctx) {
  char line[MAX_LINE];

  while (fgets(line, sizeof(line), file)) {
    line[strcspn(line, "\n")] = '\0';

    LineType line_type = classify_line_type(line, ctx);
    switch (line_type) {
    case LINE_TYPE_CODE_BLOCK_OPEN:
      if (flush_paragraph(ctx) < 0) {
        return -1;
      }

      // TODO: parse lang and put into code_block.meta
      break;
    case LINE_TYPE_CODE_BLOCK: {
      handle_code_block_line(ctx, line);
      break;
    }
    case LINE_TYPE_CODE_BLOCK_CLOSE: {
      if (handle_code_block_close(ctx) < 0) {
        return -1;
      }
    }
    case LINE_TYPE_BLANK: {
      if (flush_paragraph(ctx) < 0 || flush_block_quote(ctx) < 0) {
        return -1;
      }
      break;
    }
    case LINE_TYPE_THEMATIC_BREAK:
      if (flush_paragraph(ctx) < 0 || flush_list(ctx) < 0 ||
          flush_block_quote(ctx) < 0) {
        return -1;
      }

      if (handle_thematic_break(ctx) < 0) {
        return -1;
      }
      break;
    case LINE_TYPE_BLOCK_QUOTE: {
      if (flush_paragraph(ctx) < 0 || flush_list(ctx) < 0) {
        return -1;
      }

      if (handle_block_quote(ctx, line) < 0) {
        return -1;
      }
      break;
    }
    case LINE_TYPE_HEADING:
      if (flush_paragraph(ctx) < 0 || flush_list(ctx) < 0 ||
          flush_block_quote(ctx) < 0) {
        return -1;
      }

      if (heading_block_start(line, ctx) < 0) {
        return -1;
      }
      break;
    case LINE_TYPE_LIST: {
      if (flush_paragraph(ctx) < 0 || flush_block_quote(ctx) < 0) {
        return -1;
      }

      if (handle_list_item(ctx, line) < 0) {
        return -1;
      }
      break;
    }
    case LINE_TYPE_PARAGRAPH: {
      if (handle_paragraph_line(ctx, line) < 0) {
        return -1;
      }
    }
    default:
      break;
    }
  }

  flush_remaining_blocks(ctx);
  return 0;
}
