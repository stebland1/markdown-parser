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

TEST(unordered_list) {
  ParserContext ctx;
  parse_input("- one\n- two\n\n- three\n", &ctx);

  ASSERT_SIZE_EQUALS(1, NUM_CHILDREN(ctx.ast));
  Token *list = CHILD(ctx.ast, 0);
  ASSERT_SIZE_EQUALS(3, NUM_CHILDREN(list));
  ASSERT_STR_EQ("one", CHILD_CONTENT(list, 0));
  ASSERT_STR_EQ("two", CHILD_CONTENT(list, 1));
  ASSERT_STR_EQ("three", CHILD_CONTENT(list, 2));

  free_parser_context(&ctx);

  return 0;
}

TEST(multi_unordered_list) {
  ParserContext ctx;
  parse_input("- one\n+ two\n\n* three\n", &ctx);

  ASSERT_SIZE_EQUALS(3, NUM_CHILDREN(ctx.ast));
  ASSERT_STR_EQ("one", CHILD_CONTENT(CHILD(ctx.ast, 0), 0));
  ASSERT_STR_EQ("two", CHILD_CONTENT(CHILD(ctx.ast, 1), 0));
  ASSERT_STR_EQ("three", CHILD_CONTENT(CHILD(ctx.ast, 2), 0));

  free_parser_context(&ctx);

  return 0;
}

TEST(list_indentation) {
  ParserContext ctx;
  parse_input("- one\n  - two\n\n    - three\n", &ctx);

  ASSERT_SIZE_EQUALS(1, NUM_CHILDREN(ctx.ast));

  Token *list = CHILD(ctx.ast, 0);
  ASSERT_INT_EQUALS(LIST, TYPE(list));
  ASSERT_SIZE_EQUALS(1, NUM_CHILDREN(list));

  Token *list_item = CHILD(list, 0);
  ASSERT_INT_EQUALS(LIST_ITEM, TYPE(list_item));
  ASSERT_STR_EQ("one", CONTENT(list_item));
  ASSERT_SIZE_EQUALS(1, NUM_CHILDREN(list_item));

  Token *nested_list = CHILD(list_item, 0);
  ASSERT_INT_EQUALS(LIST, TYPE(nested_list));
  ASSERT_SIZE_EQUALS(1, NUM_CHILDREN(nested_list));

  Token *list_item_2 = CHILD(nested_list, 0);
  ASSERT_INT_EQUALS(LIST_ITEM, TYPE(list_item_2));
  ASSERT_STR_EQ("two", CONTENT(list_item_2));
  ASSERT_SIZE_EQUALS(1, NUM_CHILDREN(list_item_2));

  Token *nested_list_2 = CHILD(list_item_2, 0);
  ASSERT_INT_EQUALS(LIST, TYPE(nested_list_2));
  ASSERT_SIZE_EQUALS(1, NUM_CHILDREN(nested_list_2));

  Token *list_item_3 = CHILD(nested_list_2, 0);
  ASSERT_INT_EQUALS(LIST_ITEM, TYPE(list_item_3));
  ASSERT_STR_EQ("three", CONTENT(list_item_3));
  ASSERT_SIZE_EQUALS(0, NUM_CHILDREN(list_item_3));

  free_parser_context(&ctx);

  return 0;
}

TEST(list_complex_indentation) {
  ParserContext ctx;
  parse_input("- one\n  - two\n\n- three\n", &ctx);

  ASSERT_SIZE_EQUALS(1, NUM_CHILDREN(ctx.ast));

  Token *list = CHILD(ctx.ast, 0);
  ASSERT_INT_EQUALS(LIST, TYPE(list));
  ASSERT_SIZE_EQUALS(2, NUM_CHILDREN(list));

  Token *list_item = CHILD(list, 0);
  ASSERT_INT_EQUALS(LIST_ITEM, TYPE(list_item));
  ASSERT_STR_EQ("one", CONTENT(list_item));
  ASSERT_SIZE_EQUALS(1, NUM_CHILDREN(list_item));

  Token *nested_list = CHILD(list_item, 0);
  ASSERT_INT_EQUALS(LIST, TYPE(nested_list));
  ASSERT_SIZE_EQUALS(1, NUM_CHILDREN(nested_list));

  Token *nested_list_item = CHILD(nested_list, 0);
  ASSERT_INT_EQUALS(LIST_ITEM, TYPE(nested_list_item));
  ASSERT_STR_EQ("two", CONTENT(nested_list_item));
  ASSERT_SIZE_EQUALS(0, NUM_CHILDREN(nested_list_item));

  Token *list_item_2 = CHILD(list, 1);
  ASSERT_INT_EQUALS(LIST_ITEM, TYPE(list_item_2));
  ASSERT_STR_EQ("three", CONTENT(list_item_2));
  ASSERT_SIZE_EQUALS(0, NUM_CHILDREN(list_item_2));

  free_parser_context(&ctx);

  return 0;
}
