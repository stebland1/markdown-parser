#ifndef FRONT_MATTER_H
#include <stddef.h>
#include <stdio.h>

#define FRONT_MATTER_H

#define PARSE_ERROR -1
#define PARSE_SKIP 1
#define PARSE_OK 0

typedef struct {
  char *key;
  enum { STRING_VAL, LIST_VAL } type;
  union {
    char *string_value;
    struct {
      char **items;
      size_t count;
      size_t capacity;
    } list_value;
  };
} FrontMatterEntry;

typedef struct {
  FrontMatterEntry *entries;
  size_t count;
  size_t capacity;
} FrontMatterList;

FrontMatterList *create_front_matter_list();
void free_front_matter_list(FrontMatterList *list);
int insert_front_matter_entry(FrontMatterList *list, FrontMatterEntry *entry);
int parse_front_matter_entry(FrontMatterEntry *cur, char *line);
void print_front_matter(FrontMatterList *list);
int parse_front_matter_file(FILE *file, FrontMatterList *list);

#endif
