#include "markdown.h"
#include "blocks/heading.h"
#include "blocks/paragraph.h"
#include "inline.h"
#include "token.h"
#include "utils/stack.h"
#include "utils/utils.h"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define FRONT_MATTER_DELIM "---"
#define LINE_GROWTH_FACTOR 6
#define MAX_LINE 256

int process_file(FILE *file, ParserContext *ctx) {
  char line[MAX_LINE];

  while (fgets(line, sizeof(line), file)) {
    if (is_front_matter(line, ctx) == 1)
      continue;

    line[strcspn(line, "\n")] = '\0';

    if (is_blank_line(line)) {
      if (handle_blank_line(ctx) < 0) {
        return -1;
      }
      continue;
    }

    if (*line == '#') {
      if (heading_block_start(line, ctx) < 0) {
        return -1;
      }
      continue;
    }

    Token **active_block = peek_stack(ctx->block_stack);
    if (active_block && (*active_block)->type == DOCUMENT &&
        paragraph_block_start(ctx) < 0) {
      return -1;
    }

    active_block = peek_stack(ctx->block_stack);
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
  }

  return 0;
}

int is_front_matter(char *line, ParserContext *ctx) {
  if (!ctx->in_front_matter && strncmp(line, FRONT_MATTER_DELIM, 3) == 0) {
    ctx->in_front_matter = 1;
    return 1;
  }

  if (ctx->in_front_matter) {
    if (strncmp(line, FRONT_MATTER_DELIM, 3) == 0) {
      ctx->in_front_matter = 0;
    }
    return 1;
  }

  return 0;
}

int handle_blank_line(ParserContext *ctx) {
  Token **parent_block_ptr = peek_stack(ctx->block_stack);
  if (!parent_block_ptr) {
    return -1;
  }

  Token *parent_block = *parent_block_ptr;
  // Ensures we don't pop the root AST node from the stack.
  if (parent_block->type == DOCUMENT) {
    return 0;
  }

  if (pop(ctx->block_stack, &parent_block) < 0) {
    return -1;
  }

  if (add_child_to_token(ctx->ast, parent_block) < 0) {
    return -1;
  }

  return 0;
}
