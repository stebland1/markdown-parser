#include "front_matter.h"
#include "front_matter/entries.h"
#include "front_matter/list.h"
#include "utils/utils.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#define LIST_VALUE_CAPACITY_BUF 4
#define MAX_LINES 256

int parse_front_matter_entry(FrontMatterEntry *entry, char *line) {
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

  entry->key = escaped_key;

  char *value = delimiter + 1;
  trim(value);

  if (*value == '\0') {
    init_list_entry(entry);
    return PARSE_OK;
  }

  value = strip_double_quotes(value);
  char *escaped_val = escape_json_str(value);
  if (!escaped_val) {
    free(entry->key);
    return PARSE_ERROR;
  }

  entry->string_value = escaped_val;
  entry->type = STRING_VAL;
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

ParseState handle_idle(char *line) {
  if (*line == '\0') {
    return IDLE;
  }

  if (strcmp(line, "---") == 0) {
    return PARSING;
  }

  return DONE;
};

int handle_parsing(ParseState *state, FrontMatterEntries *entries, char *line) {
  if (strcmp(line, "---") == 0) {
    *state = DONE;
    return 0;
  }

  if (entries->count > 0 &&
      entries->items[entries->count - 1].type == LIST_VAL) {
    char *list_item = get_list_item(line);
    if (list_item) {
      FrontMatterEntry *list_entry = &entries->items[entries->count - 1];
      if (add_list_item(list_entry, list_item) < 0) {
        fprintf(stderr, "Failed to add list item\n");
        return -1;
      }

      return 0;
    }
    // fall through
  }

  FrontMatterEntry entry;
  switch (parse_front_matter_entry(&entry, line)) {
  case PARSE_SKIP:
    return 0;
  case PARSE_ERROR:
    return -1;
  }

  if (insert_front_matter_entry(entries, &entry) < 0) {
    fprintf(stderr, "Failed to insert front matter entry\n");
    return -1;
  }

  return 0;
}

int parse_front_matter_file(FILE *file, FrontMatterEntries *entries) {
  ParseState state = IDLE;
  char line[MAX_LINES];

  while (fgets(line, sizeof(line), file)) {
    line[strcspn(line, "\n")] = '\0';
    trim(line);

    switch (state) {
    case IDLE:
      state = handle_idle(line);
      break;
    case PARSING:
      if (handle_parsing(&state, entries, line) < 0) {
        return -1;
      }
      break;
    case DONE:
      break;
    }

    if (state == DONE) {
      break;
    }
  }

  return 0;
}
