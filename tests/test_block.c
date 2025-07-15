#include "context.h"
#include "parser.h"
#include "test.h"
#include "test_helpers.h"
#include "token.h"
#include "utils/debug.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

void parse_input(const char *input, ParserContext *ctx) {
  int result = init_parser_context(ctx);
  assert(result == 0);

  FILE *fp = fmemopen((void *)input, strlen(input), "r");
  assert(fp != NULL);

  int status = parse_file(fp, ctx);
  assert(status == 0);
}

TEST(paragraph) {
  ParserContext ctx;
  parse_input("para with\nmultiple\nlines\n", &ctx);

  ASSERT_SIZE_EQUALS(1, NUM_CHILDREN(ctx.ast));
  Token *para = CHILD(ctx.ast, 0);
  ASSERT_SIZE_EQUALS(3, NUM_CHILDREN(para));
  ASSERT_STR_EQ("para with", CHILD_CONTENT(CHILD(para, 0), 0));
  ASSERT_STR_EQ("multiple", CHILD_CONTENT(CHILD(para, 1), 0));
  ASSERT_STR_EQ("lines", CHILD_CONTENT(CHILD(para, 2), 0));

  free_parser_context(&ctx);

  return 0;
}
