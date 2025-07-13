#include "inline/parser.h"
#include "test.h"
#include "token.h"
#include <assert.h>
#include <string.h>

TEST(parse_single_word) {
  char *input = "word";
  Token *line_token = create_token(LINE, 1, NULL);
  int result = parse_line(input, line_token);

  assert(result == 0);
  assert(line_token->child_count == 1);
  assert(line_token->children[0]->type == TEXT);
  assert(strcmp(line_token->children[0]->content, "word") == 0);

  free_token(line_token);

  return 0;
}
