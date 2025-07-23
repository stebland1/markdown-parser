#include "front_matter/entries.h"
#include <stdio.h>

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
