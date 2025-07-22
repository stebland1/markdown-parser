#ifndef FRONT_MATTER_H
#define FRONT_MATTER_H

#include "front_matter/entries.h"
#include <stddef.h>
#include <stdio.h>

#define PARSE_ERROR -1
#define PARSE_SKIP 1
#define PARSE_OK 0

typedef enum {
  IDLE,
  PARSING,
  DONE,
} ParseState;

void print_front_matter(FrontMatterEntries *entries);
int parse_front_matter_file(FILE *file, FrontMatterEntries *entries);

#endif
