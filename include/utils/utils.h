#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))

void trim(char *str);
char *escape_json_str(char *str);
int is_blank_line(char *line);
void reverse_list(void *list, size_t list_size, size_t item_size);
char *concat(int n, ...);
char *strip_double_quotes(char *str);

#endif
