#ifndef FRONT_MATTER_LIST_H
#define FRONT_MATTER_LIST_H

#include "front_matter/entries.h"

char *get_list_item(char *line);
int add_list_item(FrontMatterEntry *entry, char *item);

#endif
