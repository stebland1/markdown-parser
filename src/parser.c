#include "parser.h"
#include "blocks/heading.h"
#include "blocks/list.h"
#include "blocks/paragraph.h"
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

int flush_current_active_block(ParserContext *ctx) {
  Token *active_block = peek_stack_value(ctx->block_stack);
  printf("Flushing item: %s\n", get_token_type_str(active_block->type));

  // Ensures we don't pop the root AST node from the stack.
  if (active_block->type == DOCUMENT) {
    return 0;
  }

  if (pop(ctx->block_stack, &active_block) < 0) {
    return -1;
  }

  if (add_child_to_token(ctx->ast, active_block) < 0) {
    return -1;
  }

  return 0;
}

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

  ListData list_data;
  if (get_list_item(line, &list_data)) {
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
      Token *active_block = peek_stack_value(ctx->block_stack);
      if (active_block->type == PARAGRAPH &&
          flush_current_active_block(ctx) < 0) {
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
      Token *active_block = peek_stack_value(ctx->block_stack);
      if (active_block->type != LIST && flush_current_active_block(ctx) < 0) {
        return -1;
      }

      ListData list_data;
      active_block = peek_stack_value(ctx->block_stack);
      char *list_item = get_list_item(line, &list_data);

      if (active_block->type == LIST &&
          (list_data.symbol != active_block->meta->list.symbol ||
           list_data.indentation != active_block->meta->list.indentation)) {
        if (flush_current_active_block(ctx) < 0) {
          return -1;
        }
      }

      active_block = peek_stack_value(ctx->block_stack);
      if (active_block->type != LIST && list_block_start(&list_data, ctx) < 0) {
        return -1;
      }

      if (add_list_item_to_list(list_item, ctx) < 0) {
        return -1;
      }

      break;
    }
    case LINE_TYPE_PARAGRAPH: {
      Token *active_block = peek_stack_value(ctx->block_stack);
      if (active_block->type != PARAGRAPH) {
        if (flush_current_active_block(ctx) < 0) {
          return -1;
        }
        if (paragraph_block_start(ctx) < 0) {
          return -1;
        }
      }

      active_block = peek_stack_value(ctx->block_stack);
      assert(active_block != NULL);
      assert(active_block->type != DOCUMENT);

      Token *line_tok = create_token(LINE, LINE_GROWTH_FACTOR, NULL, NULL);
      if (!line_tok) {
        return -1;
      }

      if (parse_line(line, line_tok) < 0) {
        free_token(line_tok);
        return -1;
      }

      if (add_child_to_token(active_block, line_tok) < 0) {
        return -1;
      }
      break;
    }
    }
  }

  return 0;
}
