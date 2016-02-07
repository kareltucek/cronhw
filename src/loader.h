#ifndef LOADER_GUARD
#define LOADER_GUARD
#include "task.h"
#include "common.h"

extern tasklist_t load_from_file(char * filename);
extern void tasklist_clear(tasklist_t tasklist);

#endif
