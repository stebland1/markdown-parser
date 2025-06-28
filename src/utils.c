#include <ctype.h>
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
