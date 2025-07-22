#include "front_matter/entries.h"
#include <stdlib.h>

FrontMatterEntries *create_front_matter_entries() {
  FrontMatterEntries *list = malloc(sizeof(FrontMatterEntries));
  if (!list) {
    return NULL;
  }

  list->capacity = LIST_ITEM_CAPACITY_BUF;
  list->count = 0;
  list->items = malloc(sizeof(FrontMatterEntry) * list->capacity);
  if (!list->items) {
    free(list);
    return NULL;
  }

  return list;
}

void free_front_matter_entries(FrontMatterEntries *list) {
  if (!list)
    return;

  for (size_t i = 0; i < list->count; i++) {
    free(list->items[i].key);

    if (list->items[i].type == STRING_VAL) {
      free(list->items[i].string_value);
    } else {
      for (size_t j = 0; j < list->items[i].list_value.count; j++) {
        free(list->items[i].list_value.items[j]);
      }
      free(list->items[i].list_value.items);
    }
  }

  free(list->items);
  free(list);
}

int insert_front_matter_entry(FrontMatterEntries *list,
                              FrontMatterEntry *entry) {
  if (list->count + 1 > list->capacity) {
    list->capacity += LIST_ITEM_CAPACITY_BUF;
    list->items =
        realloc(list->items, sizeof(FrontMatterEntry) * list->capacity);
    if (!list->items) {
      return -1;
    }
  }

  list->items[list->count++] = *entry;
  return 0;
}
