#include "parser.h"
#include "blocks/heading.h"
#include "blocks/list.h"
#include "blocks/paragraph.h"
#include "blocks/stack.h"
#include "context.h"
#include "inline/parser.h"
#include "token.h"
#include "utils/debug.h"
#include "utils/stack.h"
#include <assert.h>
#include <ctype.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define FRONT_MATTER_DELIM "---"
#define LINE_GROWTH_FACTOR 6
#define MAX_LINE 256

typedef enum {
  LINE_TYPE_FRONT_MATTER,
  LINE_TYPE_BLANK,
  LINE_TYPE_HEADING,
  LINE_TYPE_PARAGRAPH,
  LINE_TYPE_LIST,
} LineType;

int classify_line_type(char *line, ParserContext *ctx) {
  if (*line == '#') {
    return LINE_TYPE_HEADING;
  }

  char *p = line;
  while (isspace(*line))
    line++;
  if (*p == '\0') {
    return LINE_TYPE_BLANK;
  }

  if (parse_list_item(line, NULL)) {
    return LINE_TYPE_LIST;
  }

  if (!ctx->in_front_matter && strncmp(line, FRONT_MATTER_DELIM, 3) == 0) {
    ctx->in_front_matter = 1;
    return LINE_TYPE_FRONT_MATTER;
  }

  if (ctx->in_front_matter) {
    if (strncmp(line, FRONT_MATTER_DELIM, 3) == 0) {
      ctx->in_front_matter = 0;
    }
    return LINE_TYPE_FRONT_MATTER;
  }

  return LINE_TYPE_PARAGRAPH;
};

int parse_file(FILE *file, ParserContext *ctx) {
  char line[MAX_LINE];

  while (fgets(line, sizeof(line), file)) {
    line[strcspn(line, "\n")] = '\0';

    LineType line_type = classify_line_type(line, ctx);
    switch (line_type) {
    case LINE_TYPE_FRONT_MATTER:
      break;
    case LINE_TYPE_BLANK: {
      if (flush_paragraph(ctx) < 0) {
        return -1;
      }
      break;
    }
    case LINE_TYPE_HEADING:
      if (heading_block_start(line, ctx) < 0) {
        return -1;
      }
      break;
    case LINE_TYPE_LIST: {
      if (flush_paragraph(ctx) < 0) {
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
