#include "parser.h"
#include "blocks/blank_line.h"
#include "blocks/heading.h"
#include "blocks/paragraph.h"
#include "inline.h"
#include "token.h"
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
    case LINE_TYPE_BLANK:
      if (blank_line_block_start(ctx) < 0) {
        return -1;
      }
      break;
    case LINE_TYPE_HEADING:
      if (heading_block_start(line, ctx) < 0) {
        return -1;
      }
      break;
    case LINE_TYPE_PARAGRAPH: {
      Token **active_block = peek_stack(ctx->block_stack);
      if (active_block && (*active_block)->type == DOCUMENT &&
          paragraph_block_start(ctx) < 0) {
        return -1;
      }
      // fall through!
    }
    default: {
      Token **active_block = peek_stack(ctx->block_stack);
      assert(active_block != NULL);
      assert((*active_block)->type != DOCUMENT);

      Token *line_tok = create_token(LINE, LINE_GROWTH_FACTOR, NULL);
      if (!line_tok) {
        return -1;
      }

      if (parse_line(line, line_tok) < 0) {
        free_token(line_tok);
        return -1;
      }

      if (add_child_to_token(*active_block, line_tok) < 0) {
        return -1;
      }
      break;
    }
    }
  }

  return 0;
}
