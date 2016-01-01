#include "executor.h"
#include "common.h"
#include <time.h>
#include <unistd.h> 
#include "task.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h> 
#include <stdio.h>

/**
 * Executor contains method for filtering end executing the commands. Functions in this file
 * are triggered by the daemon. For more information on the data flow see the main/daemon 
 * documentation.
 * */

/**
 * Checks whether intlist contains integer a.
 * */
bool contains(intlist_t* intlist, int a)
{
  SLIST_NODE_T(intlist_t)* ptr;
  SLIST_FOREACH(ptr, intlist, entry)
    if(ptr->data == a)
      return true;
  return false;
}

/**
 * Returns the field of supplemented time which corresponds to the ith field of crontab format
 * */
int get_current_time(time_t t, int i)
{
  struct tm *info;
  info = localtime( &t );
  switch(i)
  {
    case 0:
      return info->tm_min;
      break;
    case 1:
      return info->tm_hour;
      break;
    case 2:
      return info->tm_mday;
      break;
    case 3:
      return info->tm_mon;
      break;
    case 4:
      return info->tm_wday;
      break;
  }
  return -42;
}

/**
 * constructs name for a temporary file
 * */
char* get_tmp_name(int ia, int ib)
{
  char* a = "/tmp/mycron_";
  char* b = itoa(ia);
  char* c = "_";
  char* d = itoa(ib);
  char* ab = concat(a,b);
  char* cd = concat(c,d);
  free(b);
  free(d);
  char* res = concat(ab, cd);
  free(ab);
  free(cd);
  return res;
}

/**
 * This executes the supplemented task in a newly spawned shell. The output is redirected to a 
 * temporary file, which is later read from the main thread.
 * */
void execute_task(task_t task)
{
  //printf("> execuntig something:\n"); 
  //printf("> %s\n", task.cmd);
  int ppid = getpid();
  int pid = fork();
  if(pid < 0)
    stderror(STD_ERR);
  if(pid == 0)
  {
    int mypid = getpid();
    char* name = get_tmp_name(ppid, mypid);
    int errfd = dup(STD_ERR);
    if(close(STD_ERR) < 0 ) stderror(errfd);
    if(open(name,O_WRONLY | O_CREAT, 0660) < 0) stderror(errfd);
    if(close(STD_OUT) < 0) stderror(errfd);
    if(dup(STD_ERR) < 0) stderror(errfd);
    free(name);
    char* argv[] = {"/bin/sh", "-c", task.cmd, NULL};
    char* msg = "executing command "; write(STD_OUT, msg, strlen(msg));
    write(STD_OUT, task.cmd, strlen(task.cmd));
    char* nl = "\n"; write(STD_OUT, nl, strlen(nl));
    if(!execv("/bin/sh", argv)) stderror(errfd);
  }
}

/**
 * Gets one task and decides whether it is to be executed. If so, it initiates the execution.
 * */
void handle_task(task_t task, time_t t)
{
  //printf("> checking command: %s\n", task.cmd);
  for(int i = 0; i < 5; i++)
  {
    int want = get_current_time(t, i);
    if(!contains(&task.times[i], want) && !contains(&task.times[i], -1))
      return;
  }
  execute_task(task);
}

/**
 * Goes through tasks, decides which are to be executed and triggers their execution.
 * */
void execute_tasks(tasklist_t tasklist, time_t t)
{
  //printf("> checking executable tasks\n"); 
  SLIST_NODE_T(tasklist_t)* ptr;
  SLIST_FOREACH(ptr, &tasklist, entry)
    handle_task(ptr->data, t);
}

