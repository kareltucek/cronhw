#ifndef LOADER_GUARD
#define LOADER_GUARD
#include "task.h"
#include "common.h"

extern tasklist_t loadFromFile(char* filename);
extern void tasklist_clear(tasklist_t tasklist);

#endif
