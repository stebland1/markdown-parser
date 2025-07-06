#include "markdown.h"
#include <stdio.h>
#include <stdlib.h>

#define MAX_LINE 256

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

  char line[MAX_LINE];
  int in_front_matter = 0;

  while (fgets(line, sizeof(line), file)) {
    if (skip_line(line, &in_front_matter) == 1) {
      continue;
    }
  }

  fclose(file);
  return EXIT_SUCCESS;
}
