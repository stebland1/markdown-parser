#include "front_matter/entries.h"
#include "utils/utils.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

void init_list_entry(FrontMatterEntry *entry) {
  entry->type = LIST_VAL;
  entry->list_value.items = NULL;
  entry->list_value.count = 0;
  entry->list_value.capacity = 0;
}

char *get_list_item(char *line) {
  char *p = line;
  if (*p++ != '-')
    return NULL;
  while (isspace(*p))
    p++;

  trim(p);
  return strip_double_quotes(p);
}

int add_list_item(FrontMatterEntry *entry, char *item) {
  if (entry->list_value.count == entry->list_value.capacity) {
    entry->list_value.capacity += LIST_VALUE_CAPACITY_BUF;
    entry->list_value.items = (char **)realloc(
        entry->list_value.items, entry->list_value.capacity * sizeof(char *));

    if (!entry->list_value.items) {
      fprintf(stderr, "Failed to allocate for list items\n");
      return -1;
    }
  }

  char *escaped_list_item = escape_json_str(item);
  if (!escaped_list_item) {
    fprintf(stderr, "Failed to allocate for list item\n");
    return -1;
  }

  entry->list_value.items[entry->list_value.count++] = escaped_list_item;
  return 0;
}
