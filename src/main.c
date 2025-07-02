#include "front_matter.h"
#include "utils.h"
#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#define MAX_LINES 256
#define LIST_VALUE_CAPACITY_BUF 4

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
