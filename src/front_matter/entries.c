#include "front_matter/entries.h"
#include <stdlib.h>

FrontMatterList *create_front_matter_list() {
  FrontMatterList *list = malloc(sizeof(FrontMatterList));
  if (!list) {
    return NULL;
  }

  list->capacity = LIST_ITEM_CAPACITY_BUF;
  list->count = 0;
  list->entries = malloc(sizeof(FrontMatterEntry) * list->capacity);
  if (!list->entries) {
    free(list);
    return NULL;
  }

  return list;
}

void free_front_matter_list(FrontMatterList *list) {
  if (!list)
    return;

  for (size_t i = 0; i < list->count; i++) {
    free(list->entries[i].key);

    if (list->entries[i].type == STRING_VAL) {
      free(list->entries[i].string_value);
    } else {
      for (size_t j = 0; j < list->entries[i].list_value.count; j++) {
        free(list->entries[i].list_value.items[j]);
      }
      free(list->entries[i].list_value.items);
    }
  }

  free(list->entries);
  free(list);
}

int insert_front_matter_entry(FrontMatterList *list, FrontMatterEntry *entry) {
  if (list->count + 1 > list->capacity) {
    list->capacity += LIST_ITEM_CAPACITY_BUF;
    list->entries =
        realloc(list->entries, sizeof(FrontMatterEntry) * list->capacity);
    if (!list->entries) {
      return -1;
    }
  }

  list->entries[list->count++] = *entry;
  return 0;
}
