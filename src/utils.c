#include "utils.h"
#include "inline.h"
#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void trim(char *str) {
  char *start = str;
  for (; isspace(*start); start++)
    ;

  if (*start == '\0') {
    *str = '\0';
    return;
  }

  char *end = str + strlen(str) - 1;
  for (; end > start && isspace(*end); end--)
    ;
  *(end + 1) = '\0';

  if (start > str) {
    memmove(str, start, end - start + 2);
  }
}

char *escape_json_str(char *input) {
  size_t len = strlen(input);
  char *out = malloc(len * 6 + 1);

  if (!out)
    return NULL;

  char *p = out;

  for (size_t i = 0; i < len; i++) {
    unsigned char c = input[i];

    switch (c) {
    case '\"':
      *p++ = '\\';
      *p++ = '\"';
      break;
    case '\\':
      *p++ = '\\';
      *p++ = '\\';
      break;
    case '\n':
      *p++ = '\\';
      *p++ = 'n';
      break;
    case '\t':
      *p++ = '\\';
      *p++ = 't';
      break;
    case '\b':
      *p++ = '\\';
      *p++ = 'b';
      break;
    case '\f':
      *p++ = '\\';
      *p++ = 'f';
      break;
    case '\r':
      *p++ = '\\';
      *p++ = 'r';
      break;
    default:
      if (c < 0x20) {
        p += sprintf(p, "\\u%04x", c);
      } else {
        *p++ = c;
      }
      break;
    }
  }

  *p = '\0';
  return out;
}

int is_blank_line(char *line) {
  while (isspace(*line))
    line++;
  return *line == '\0';
}

void reverse_list(void *list, size_t list_size, size_t item_size) {
  if (list_size <= 1) {
    return;
  }

  char *arr = (char *)list;
  void *tmp = malloc(list_size * item_size);
  if (!tmp) {
    return;
  }

  for (size_t i = 0; i < list_size / 2; i++) {
    size_t j = list_size - 1 - i;

    memcpy(tmp, arr + i * item_size, item_size);
    memcpy(arr + i * item_size, arr + j * item_size, item_size);
    memcpy(arr + j * item_size, tmp, item_size);
  }

  free(tmp);
}
