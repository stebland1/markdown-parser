#include "blocks/list.h"
#include "parser.h"
#include "token.h"
#include <assert.h>
#include <ctype.h>

char *get_list_item(char *c, ListData *data) {
  int indentation = 0;
  while (isspace(*c)) {
    indentation++;
    c++;
  }

  while (*c) {
    switch (*c) {
    case '-':
    case '+':
    case '*':
      data->symbol = *c;
      c++;
      while (isspace(*c)) {
        c++;
      }

      if (*c == '\0') {
        return NULL;
      }

      data->indentation = indentation;
      return c;
    default:
      return NULL;
    }
  }

  return c;
}

int list_block_start(ListData *data, ParserContext *ctx) {
  Token *list_token = create_token(LIST, LIST_GROWTH_FACTOR, NULL, data);
  if (!list_token) {
    return -1;
  }

  if (push(ctx->block_stack, &list_token) < 0) {
    free_token(list_token);
    return -1;
  }

  return 0;
}

int add_list_item_to_list(char *list_item, ParserContext *ctx) {
  Token *active_block = peek_stack_value(ctx->block_stack);

  assert(list_item != NULL);
  assert(active_block->type == LIST);

  // TODO: keep track of the type of list i.e. ordered, unordered
  // and what symbol started the list i.e. - * +
  Token *list_item_token = create_token(LIST_ITEM, 0, list_item, NULL);
  if (add_child_to_token(active_block, list_item_token) < 0) {
    return -1;
  }

  return 0;
}
