#ifndef FRONT_MATTER_PARSER_H
#define FRONT_MATTER_PARSER_H

#include "front_matter/entries.h"

#define PARSE_ERROR -1
#define PARSE_SKIP 1
#define PARSE_OK 0

typedef enum {
  IDLE,
  PARSING,
  DONE,
} ParseState;

int parse_key(char *key, FrontMatterEntry *entry);
int parse_string_value(char *value, FrontMatterEntry *entry);
int parse_front_matter_entry(FrontMatterEntry *entry, char *line);
void print_front_matter(FrontMatterEntries *entries);
ParseState handle_idle(char *line);

#endif
