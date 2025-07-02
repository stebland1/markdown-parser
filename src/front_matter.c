#include "front_matter.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LIST_ITEM_CAPACITY_BUF 8

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

int parse_front_matter_entry(FrontMatterEntry *cur, char *line) {
  char *delimiter = strchr(line, ':');
  if (delimiter == NULL) {
    return PARSE_SKIP;
  }

  *delimiter = '\0';
  trim(line);
  char *escaped_key = escape_json_str(line);
  if (!escaped_key) {
    return PARSE_ERROR;
  }

  cur->key = escaped_key;

  char *value = delimiter + 1;
  value[strcspn(value, "\n")] = '\0';
  trim(value);

  if (*value == '\0') {
    cur->type = LIST_VAL;
    return PARSE_OK;
  }

  char *escaped_val = escape_json_str(value);
  if (!escaped_val) {
    free(cur->key);
    return PARSE_ERROR;
  }

  cur->string_value = escaped_val;
  cur->type = STRING_VAL;
  return PARSE_OK;
}

void print_front_matter(FrontMatterList *list) {
  printf("{");
  for (size_t i = 0; i < list->count; i++) {
    FrontMatterEntry entry = list->entries[i];
    printf("\"%s\": ", entry.key);

    switch (entry.type) {
    case LIST_VAL:
      printf("[");
      for (size_t j = 0; j < entry.list_value.count; j++) {
        printf("\"%s\"%s", entry.list_value.items[j],
               j < entry.list_value.count - 1 ? "," : "");
      }
      printf("]");
      break;
    case STRING_VAL:
      printf("\"%s\"%s", entry.string_value, i < list->count - 1 ? "," : "");
      break;
    }
  }
  printf("}");
}
