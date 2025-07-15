#include "blocks/list.h"
#include "blocks/stack.h"
#include "context.h"
#include "token.h"
#include "utils/stack.h"
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
      data->parent = 0;
      return c;
    default:
      return NULL;
    }
  }

  return c;
}

Token *create_list_token(ParserContext *ctx, char *content, ListData *meta) {
  Token *list_item_token = create_token(LIST_ITEM, 0, content, NULL);
  if (!list_item_token) {
    return NULL;
  }

  meta->last = list_item_token;
  Token *list_token = create_token(LIST, LIST_GROWTH_FACTOR, NULL, meta);
  if (!list_token) {
    free_token(list_item_token);
    return NULL;
  }

  if (add_child_to_token(list_token, list_item_token) < 0) {
    free_token(list_token);
    free_token(list_item_token);
    return NULL;
  }

  return list_token;
}

Token *push_new_list_token(ParserContext *ctx, char *list_item,
                           ListData *data) {
  Token *list_token = create_list_token(ctx, list_item, data);
  if (!list_token) {
    return NULL;
  }

  if (push(&ctx->block_stack, &list_token) < 0) {
    free_token(list_token);
    return NULL;
  }

  return list_token;
}

int is_nested_list(ListData *current, Token *active) {
  return current->indentation >= active->meta->list.indentation + 2;
}

int stop_when_indentation_is_equal(Token *token, void *userdata) {
  ListData data = *(ListData *)userdata;
  return token->type != LIST ||
         token->meta->list.indentation == data.indentation ||
         token->meta->list.indentation == data.indentation + 1;
}

int handle_list_item(ParserContext *ctx, char *line) {
  ListData cur_list_item_data;
  char *list_item = get_list_item(line, &cur_list_item_data);
  Token *active_block = peek_stack_value(&ctx->block_stack);

  if (active_block->type != LIST) {
    cur_list_item_data.parent = 1;
    if (push_new_list_token(ctx, list_item, &cur_list_item_data) == NULL) {
      return -1;
    }

    return 0;
  }

  if (cur_list_item_data.symbol != active_block->meta->list.symbol) {
    if (flush_list(ctx) < 0) {
      return -1;
    }

    cur_list_item_data.parent = 1;
    if (push_new_list_token(ctx, list_item, &cur_list_item_data) == NULL) {
      return -1;
    }

    return 0;
  }

  if (is_nested_list(&cur_list_item_data, active_block)) {
    Token *list_token =
        push_new_list_token(ctx, list_item, &cur_list_item_data);
    if (!list_token) {
      return -1;
    }

    Token *last_item = active_block->meta->list.last;
    if (add_child_to_token(last_item, list_token) < 0) {
      return -1;
    }

    return 0;
  }

  if (cur_list_item_data.indentation < active_block->meta->list.indentation) {
    if (flush_stack(ctx, stop_when_indentation_is_equal, add_parent_list_only,
                    (void *)&cur_list_item_data) < 0) {
      return -1;
    }

    active_block = peek_stack_value(&ctx->block_stack);
    if (active_block->type == LIST) {
      Token *list_item_token = create_token(LIST_ITEM, 0, list_item, NULL);
      if (add_child_to_token(active_block, list_item_token) < 0) {
        return -1;
      }

      return 0;
    }

    Token *list_token =
        push_new_list_token(ctx, list_item, &cur_list_item_data);
    if (!list_token) {
      return -1;
    }

    return 0;
  }

  Token *list_item_token = create_token(LIST_ITEM, 0, list_item, NULL);
  if (!list_item_token) {
    return -1;
  }

  if (add_child_to_token(active_block, list_item_token) < 0) {
    return -1;
  }

  active_block->meta->list.last = list_item_token;
  return 0;
}
