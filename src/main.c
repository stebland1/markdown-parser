#include "utils.h"
#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#define MAX_LINES 256
#define LIST_ITEM_CAPACITY_BUF 8
#define LIST_VALUE_CAPACITY_BUF 4

#define PARSE_ERROR -1
#define PARSE_SKIP 1
#define PARSE_OK 0

typedef struct {
  char *key;
  enum { STRING_VAL, LIST_VAL } type;
  union {
    char *string_value;
    struct {
      char **items;
      size_t count;
      size_t capacity;
    } list_value;
  };
} FrontMatterEntry;

typedef struct {
  FrontMatterEntry *entries;
  size_t count;
  size_t capacity;
} FrontMatterList;

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

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "Invalid argument count: provided %d, expected 1\n",
            argc - 1);
    return EXIT_FAILURE;
  }

  char *path = argv[1];
  FILE *file = fopen(path, "r");

  if (file == NULL) {
    fprintf(stderr, "Failed to open file: %s\n", path);
    return EXIT_FAILURE;
  }

  FrontMatterList *list = create_front_matter_list();
  if (!list) {
    fprintf(stderr, "Failed to create front matter list\n");
    fclose(file);
    return EXIT_FAILURE;
  }

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
      if (isspace(*line)) {
        char *p = line;
        while (isspace(*p))
          p++;
        if (*p == '-') {
          p++;
          while (isspace(*p))
            p++;

          FrontMatterEntry *list_entry = &list->entries[list->count - 1];
          if (list_entry->list_value.count == list_entry->list_value.capacity) {
            list_entry->list_value.capacity += LIST_VALUE_CAPACITY_BUF;
            list_entry->list_value.items = (char **)realloc(
                list_entry->list_value.items,
                list_entry->list_value.capacity * sizeof(char *));

            if (!list_entry->list_value.items) {
              fprintf(stderr, "Failed to allocate for list items\n");
              goto fail;
            }
          }

          char *list_item = p;
          list_item[strcspn(list_item, "\n")] = '\0';
          trim(list_item);
          char *escaped_list_item = escape_json_str(list_item);
          if (!escaped_list_item) {
            fprintf(stderr, "Failed to allocate for list item\n");
            goto fail;
          }
          list_entry->list_value.items[list_entry->list_value.count++] =
              escaped_list_item;
          continue;
        } else {
          is_list = 0;
        }
      } else {
        is_list = 0;
      }
    }

    FrontMatterEntry entry;
    switch (parse_front_matter_entry(&entry, line)) {
    case PARSE_SKIP:
      continue;
    case PARSE_ERROR:
      goto fail;
    }

    if (entry.type == LIST_VAL && !is_list) {
      entry.list_value.items = NULL;
      entry.list_value.count = 0;
      entry.list_value.capacity = 0;
      is_list = 1;
    }

    if (insert_front_matter_entry(list, &entry) < 0) {
      fprintf(stderr, "Failed to insert front matter entry\n");
      goto fail;
    }
  }

  print_front_matter(list);
  free_front_matter_list(list);
  fclose(file);
  return EXIT_SUCCESS;

fail:
  free_front_matter_list(list);
  fclose(file);
  return EXIT_FAILURE;
}
