#ifndef FRONT_MATTER_ENTRIES_H
#define FRONT_MATTER_ENTRIES_H

#include <stddef.h>

#define LIST_VALUE_CAPACITY_BUF 4
#define LIST_ITEM_CAPACITY_BUF 8

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
int parse_front_matter_entry(FrontMatterEntry *cur, char *line);
int insert_front_matter_entry(FrontMatterList *list, FrontMatterEntry *entry);

#endif
