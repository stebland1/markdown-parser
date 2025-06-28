#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

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

  int c;
  while ((c = fgetc(file)) != EOF) {
    printf("%c", c);
  }

  fclose(file);
}
