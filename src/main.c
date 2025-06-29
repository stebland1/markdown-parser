#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#define MAX_LINES 256
#define MAX_ENTRIES 50

typedef struct {
  char key[64];
  char value[256];
} Entry;

int parse_front_matter(Entry *cur, char *line) {
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
    printf("Invalid argument count: provided %d, expected 1\n", argc - 1);
    return EXIT_FAILURE;
  }

  char *path = argv[1];
  FILE *file = fopen(path, "r");

  if (file == NULL) {
    printf("Failed to open file: %s\n", path);
    return EXIT_FAILURE;
  }

  Entry entries[MAX_ENTRIES];
  int entry_count = 0;

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

    if (in_front_matter &&
        parse_front_matter(&entries[entry_count], line) == 0) {
      entry_count++;
    }
  }

  printf("{");
  for (int i = 0; i < entry_count; i++) {
    printf("\"%s\":\"%s\"%s", entries[i].key, entries[i].value,
           i < entry_count - 1 ? "," : "");
  }
  printf("}");

  fclose(file);
}
