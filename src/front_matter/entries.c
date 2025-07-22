#include "front_matter/entries.h"
#include <stdio.h>
#include <stdlib.h>

FrontMatterEntries *create_front_matter_entries() {
  FrontMatterEntries *entries = malloc(sizeof(FrontMatterEntries));
  if (!entries) {
    return NULL;
  }

  entries->capacity = ENTRIES_CAPACITY_BUF;
  entries->count = 0;
  entries->items = malloc(sizeof(FrontMatterEntry) * entries->capacity);
  if (!entries->items) {
    free(entries);
    return NULL;
  }

  return entries;
}

void free_front_matter_entries(FrontMatterEntries *entries) {
  if (!entries)
    return;

  for (size_t i = 0; i < entries->count; i++) {
    free(entries->items[i].key);

    if (entries->items[i].type == STRING_VAL) {
      free(entries->items[i].string_value);
    } else {
      for (size_t j = 0; j < entries->items[i].list_value.count; j++) {
        free(entries->items[i].list_value.items[j]);
      }
      free(entries->items[i].list_value.items);
    }
  }

  free(entries->items);
  free(entries);
}

int insert_front_matter_entry(FrontMatterEntries *entries,
                              FrontMatterEntry *entry) {
  if (entries->count + 1 > entries->capacity) {
    entries->capacity += ENTRIES_CAPACITY_BUF;
    entries->items =
        realloc(entries->items, sizeof(FrontMatterEntry) * entries->capacity);
    if (!entries->items) {
      return -1;
    }
  }

  entries->items[entries->count++] = *entry;
  return 0;
}
