#include "inline/parser.h"
#include "test.h"
#include "test_helpers.h"
#include "token.h"
#include <assert.h>
#include <string.h> // IWYU pragma: keep

Token *create_and_parse_line_token(const char *input) {
  Token *line_token = create_token(LINE, 1, NULL);
  int result = parse_line((char *)input, line_token);
  assert(result == 0);
  return line_token;
}

TEST(parse_single_word) {
  Token *line_token = PARSE_LINE("word");

  ASSERT_SIZE_EQUALS(1, NUM_CHILDREN(line_token));
  ASSERT_INT_EQUALS(CHILD_TYPE(line_token, 0), TEXT);
  ASSERT_STR_EQ("word", CHILD_CONTENT(line_token, 0));

  free_token(line_token);

  return 0;
}

TEST(parse_italics_asterisk) {
  Token *line_token = PARSE_LINE("*another word*");

  ASSERT_SIZE_EQUALS(1, NUM_CHILDREN(line_token));
  ASSERT_INT_EQUALS(ITALIC, CHILD_TYPE(line_token, 0));

  Token *italic_token = CHILD(line_token, 0);
  ASSERT_SIZE_EQUALS(1, NUM_CHILDREN(italic_token));
  ASSERT_INT_EQUALS(TEXT, CHILD_TYPE(italic_token, 0));
  ASSERT_STR_EQ("another word", CHILD_CONTENT(italic_token, 0));

  free_token(line_token);

  return 0;
}

TEST(parse_italics_underscore) {
  Token *line_token = PARSE_LINE("_another word_");

  ASSERT_SIZE_EQUALS(1, NUM_CHILDREN(line_token));
  ASSERT_INT_EQUALS(ITALIC, CHILD_TYPE(line_token, 0));

  Token *italic_token = CHILD(line_token, 0);
  ASSERT_SIZE_EQUALS(1, NUM_CHILDREN(italic_token));
  ASSERT_INT_EQUALS(TEXT, CHILD_TYPE(italic_token, 0));
  ASSERT_STR_EQ("another word", CHILD_CONTENT(italic_token, 0));

  free_token(line_token);

  return 0;
}

TEST(parse_bold_nested_italic) {
  Token *line_token = PARSE_LINE("**another _word_**");

  ASSERT_SIZE_EQUALS(1, NUM_CHILDREN(line_token));
  ASSERT_INT_EQUALS(BOLD, CHILD_TYPE(line_token, 0));

  Token *bold_token = CHILD(line_token, 0);
  ASSERT_SIZE_EQUALS(2, NUM_CHILDREN(bold_token));
  ASSERT_INT_EQUALS(TEXT, CHILD_TYPE(bold_token, 0));
  ASSERT_STR_EQ("another ", CHILD_CONTENT(bold_token, 0));
  ASSERT_INT_EQUALS(ITALIC, CHILD_TYPE(bold_token, 1));

  Token *italic_token = CHILD(bold_token, 1);
  ASSERT_SIZE_EQUALS(1, NUM_CHILDREN(italic_token));
  ASSERT_STR_EQ("word", CHILD_CONTENT(italic_token, 0));

  free_token(line_token);

  return 0;
}
