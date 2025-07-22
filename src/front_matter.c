#include "front_matter.h"
#include "front_matter/entries.h"
#include "utils/utils.h"
#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#define LIST_VALUE_CAPACITY_BUF 4
#define MAX_LINES 256

char *strip_double_quotes(char *str) {
  if (!str || strlen(str) < 2) {
    return str;
  }

  char *start = str;
  char *end = str + strlen(str) - 1;

  if (*start == '"' && *end == '"') {
    start++;
    *end = '\0';
  }

  return start;
}

int parse_front_matter_entry(FrontMatterEntry *cur, char *line) {
  char *delimiter = strchr(line, ':');
  if (delimiter == NULL) {
    return PARSE_SKIP;
  }

  *delimiter = '\0';
  trim(line);
  char *escaped_key = escape_json_str(line);
  if (!escaped_key) {
    return PARSE_ERROR;
  }

  cur->key = escaped_key;

  char *value = delimiter + 1;
  trim(value);

  if (*value == '\0') {
    cur->type = LIST_VAL;
    return PARSE_OK;
  }

  value = strip_double_quotes(value);
  char *escaped_val = escape_json_str(value);
  if (!escaped_val) {
    free(cur->key);
    return PARSE_ERROR;
  }

  cur->string_value = escaped_val;
  cur->type = STRING_VAL;
  return PARSE_OK;
}

void print_front_matter(FrontMatterEntries *entries) {
  printf("{");
  for (size_t i = 0; i < entries->count; i++) {
    FrontMatterEntry entry = entries->items[i];
    printf("\"%s\": ", entry.key);

    switch (entry.type) {
    case LIST_VAL:
      printf("[");
      for (size_t j = 0; j < entry.list_value.count; j++) {
        printf("\"%s\"%s", entry.list_value.items[j],
               j < entry.list_value.count - 1 ? "," : "");
      }
      printf("]");
      break;
    case STRING_VAL:
      printf("\"%s\"", entry.string_value);
      break;
    }

    if (i < entries->count - 1) {
      printf(",");
    }
  }
  printf("}");
}

char *get_list_item(char *line) {
  char *p = line;
  if (*p++ != '-')
    return NULL;
  while (isspace(*p))
    p++;

  trim(p);
  return strip_double_quotes(p);
}

int add_list_item(FrontMatterEntry *entry, char *item) {
  if (entry->list_value.count == entry->list_value.capacity) {
    entry->list_value.capacity += LIST_VALUE_CAPACITY_BUF;
    entry->list_value.items = (char **)realloc(
        entry->list_value.items, entry->list_value.capacity * sizeof(char *));

    if (!entry->list_value.items) {
      fprintf(stderr, "Failed to allocate for list items\n");
      return -1;
    }
  }

  char *escaped_list_item = escape_json_str(item);
  if (!escaped_list_item) {
    fprintf(stderr, "Failed to allocate for list item\n");
    return -1;
  }

  entry->list_value.items[entry->list_value.count++] = escaped_list_item;
  return 0;
}

int parse_front_matter_file(FILE *file, FrontMatterEntries *entries) {
  ParseState state = OUTSIDE;
  char line[MAX_LINES];

  while (fgets(line, sizeof(line), file)) {
    line[strcspn(line, "\n")] = '\0';
    trim(line);

    if (state == OUTSIDE) {
      if (*line == '\0')
        continue;
      if (strcmp(line, "---") == 0) {
        state = IN_FRONT_MATTER;
        continue;
      } else {
        break;
      }
    }

    if (strcmp(line, "---") == 0) {
      break;
    }

    if (state == IN_LIST) {
      char *list_item = get_list_item(line);
      if (list_item) {
        FrontMatterEntry *list_entry = &entries->items[entries->count - 1];
        if (add_list_item(list_entry, list_item) < 0) {
          fprintf(stderr, "Failed to add list item\n");
          return -1;
        }
        continue;
      } else {
        state = IN_FRONT_MATTER;
      }
    }

    FrontMatterEntry entry;
    switch (parse_front_matter_entry(&entry, line)) {
    case PARSE_SKIP:
      continue;
    case PARSE_ERROR:
      return -1;
    }

    if (entry.type == LIST_VAL && state != IN_LIST) {
      entry.list_value.items = NULL;
      entry.list_value.count = 0;
      entry.list_value.capacity = 0;
      state = IN_LIST;
    }

    if (insert_front_matter_entry(entries, &entry) < 0) {
      fprintf(stderr, "Failed to insert front matter entry\n");
      return -1;
    }
  }

  return 0;
}
