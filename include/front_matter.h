#ifndef FRONT_MATTER_H
#define FRONT_MATTER_H

#include "front_matter/entries.h"
#include <stddef.h>
#include <stdio.h>

#define PARSE_ERROR -1
#define PARSE_SKIP 1
#define PARSE_OK 0

typedef enum {
  OUTSIDE,
  IN_FRONT_MATTER,
  IN_LIST,
} ParseState;

void print_front_matter(FrontMatterList *list);
int parse_front_matter_file(FILE *file, FrontMatterList *list);

#endif
