#ifndef EXECUTOR_GUARD
#define EXECUTOR_GUARD
#include "task.h"
#include <time.h>
#include "common.h"

extern void execute_tasks(tasklist_t tasklist, time_t t);
extern char* get_tmp_name(int ia, int ib);

#endif
