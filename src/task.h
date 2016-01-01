#ifndef TASK_GUARD
#define TASK_GUARD

#include <stdlib.h>
#include "../third-party-src/queue.h"
#include "common.h"

#define MINUTES 0
#define HOURS 1
#define DOM 2
#define MONTH 3
#define DOW 4

/**
 * This file contains a task structure definition, which contains data
 * needed for determinin whether a task is to be run at a specific time 
 * or not. Also as in case of loader, this file contains some macros which
 * simplify usage of the queue.h macro library
 * */

#define SLIST_NODE_T_GEN(name,  type) \
  typedef struct name { \
    type data; \
    SLIST_ENTRY(name) entry; \
  } name; 

#define SLIST_NODE_T_GENC(name,  type) \
  name* name##_SLIST_CONSTRUCTOR(type d) \
  { \
    name* node = (name*)malloc(sizeof(name)); \
    node->data = d; \
    return node; \
  }

#define SLIST_LIST_T(name, type) \
  SLIST_NODE_T_GEN(name##_node_t,  type); \
  typedef SLIST_HEAD(name, name##_node_t) name;

#define SLIST_LIST_T_GEN(name, type) \
  SLIST_NODE_T_GENC(name##_node_t,  type);

#define SLIST_NODE(name, value) \
  name##_node_t_SLIST_CONSTRUCTOR(value)

#define SLIST_NODE_T(name) \
  name##_node_t


SLIST_LIST_T(intlist_t, int)


typedef struct task_t
{
  intlist_t times[5];
  char* cmd;
  char* cmdline;
} task_t;

SLIST_LIST_T(tasklist_t, task_t)


#endif
