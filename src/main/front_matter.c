#include "front_matter/entries.h"
#include "front_matter/parser.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#define MAX_LINES 256

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

  FrontMatterEntries *entries = create_front_matter_entries();
  if (!entries) {
    fprintf(stderr, "Failed to create front matter list\n");
    fclose(file);
    return EXIT_FAILURE;
  }

  if (parse_front_matter_file(file, entries) < 0) {
    free_front_matter_entries(entries);
    fclose(file);
    return EXIT_FAILURE;
  };

  print_front_matter(entries);
  free_front_matter_entries(entries);
  fclose(file);
  return EXIT_SUCCESS;
}
