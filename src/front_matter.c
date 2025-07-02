#include "front_matter.h"
#include "utils.h"
#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#define LIST_VALUE_CAPACITY_BUF 4
#define LIST_ITEM_CAPACITY_BUF 8
#define MAX_LINES 256

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

char *get_list_item(char *line) {
  if (!isspace(*line))
    return NULL;

  char *p = line;
  while (isspace(*p))
    p++;
  if (*p++ != '-')
    return NULL;
  while (isspace(*p))
    p++;

  p[strcspn(p, "\n")] = '\0';
  trim(p);
  return p;
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

int parse_front_matter_file(FILE *file, FrontMatterList *list) {
  int in_front_matter = 0;
  int is_list = 0;
  char line[MAX_LINES];

  while (fgets(line, sizeof(line), file)) {
    if (strncmp("---", line, 3) == 0) {
      if (in_front_matter) {
        break;
      } else {
        in_front_matter = 1;
      }
    }

    if (!in_front_matter)
      continue;

    if (is_list) {
      char *list_item = get_list_item(line);
      if (list_item) {
        FrontMatterEntry *list_entry = &list->entries[list->count - 1];
        if (add_list_item(list_entry, list_item) < 0) {
          fprintf(stderr, "Failed to add list item\n");
          return -1;
        }
        continue;
      } else {
        is_list = 0;
      }
    }

    FrontMatterEntry entry;
    switch (parse_front_matter_entry(&entry, line)) {
    case PARSE_SKIP:
      continue;
    case PARSE_ERROR:
      return -1;
    }

    if (entry.type == LIST_VAL && !is_list) {
      entry.list_value.items = NULL;
      entry.list_value.count = 0;
      entry.list_value.capacity = 0;
      is_list = 1;
    }

    if (insert_front_matter_entry(list, &entry) < 0) {
      fprintf(stderr, "Failed to insert front matter entry\n");
      return -1;
    }
  }

  return 0;
}
