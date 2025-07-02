#include "front_matter.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
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

  if (parse_front_matter_file(file, list) < 0) {
    free_front_matter_list(list);
    fclose(file);
    return EXIT_FAILURE;
  };

  print_front_matter(list);
  free_front_matter_list(list);
  fclose(file);
  return EXIT_SUCCESS;
}
