#include "utils.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#define MAX_LINES 256
#define MAX_ENTRIES 50

typedef struct {
  char key[64];
  char value[256];
} FrontMatterEntry;

typedef struct {
  FrontMatterEntry *entries;
  size_t count;
  size_t capacity;
} FrontMatterList;

FrontMatterList *create_front_matter_list() {
  FrontMatterList *list = malloc(sizeof(FrontMatterList));
  list->capacity = 8;
  list->count = 0;
  list->entries = malloc(sizeof(FrontMatterEntry) * list->capacity);
  return list;
}

void free_front_matter_list(FrontMatterList *list) {
  if (list) {
    free(list->entries);
    free(list);
  }
}

int insert_front_matter_entry(FrontMatterList *list, FrontMatterEntry *entry) {
  if (list->count + 1 > list->capacity) {
    list->capacity += 8;
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
    return -1;
  }

  *delimiter = '\0';
  trim(line);
  strcpy(cur->key, line);
  char *value = delimiter + 1;
  value[strcspn(value, "\n")] = '\0';
  trim(value);
  strcpy(cur->value, value);

  return 0;
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
  char line[MAX_LINES];
  while (fgets(line, sizeof(line), file)) {
    if (strncmp("---", line, 3) == 0) {
      if (in_front_matter) {
        break;
      } else {
        in_front_matter = 1;
      }
    }

    FrontMatterEntry entry;
    if (in_front_matter && parse_front_matter_entry(&entry, line) == 0) {
      if (insert_front_matter_entry(list, &entry)) {
        fprintf(stderr, "Failed to insert front matter entry\n");
        free(list);
        fclose(file);
        return EXIT_FAILURE;
      }
    }
  }

  printf("{");
  for (size_t i = 0; i < list->count; i++) {
    printf("\"%s\":\"%s\"%s", list->entries[i].key, list->entries[i].value,
           i < list->count - 1 ? "," : "");
  }
  printf("}");

  fclose(file);
  free_front_matter_list(list);
}
