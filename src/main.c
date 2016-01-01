#include "../third-party-src/stolen.h"
#include "loader.h"
#include "task.h"
#include "executor.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h> 
#include <stdio.h>

#include <sys/wait.h>
#include "common.h"

/**
 * This file contains the toplevel management stuff. 
 *
 **/

void help()
{
  char* str;
  str = "syntax: crond [OPTIONS] -f <cronfile>\n"; write(STD_OUT, str, strlen(str));
  str = "options: \n"; write(STD_OUT, str, strlen(str));
  str = "  -f <file> (for now obligatory)\n"; write(STD_OUT, str, strlen(str));
  str = "  -d        daemonize\n"; write(STD_OUT, str, strlen(str));
  str = "  -s        silent\n"; write(STD_OUT, str, strlen(str));
  str = "  -o        once (for memtest purposes)\n"; write(STD_OUT, str, strlen(str));
}

/**
 * returns current timestamp
 * */
time_t timestamp()
{
  time_t t;
  time(&t);
  return t;
}

time_t get_mtime(char* filename)
{
  struct stat st;
  if (stat(filename, &st) == -1) 
  {
    stderror(STD_ERR);
  }
  return st.st_mtime;
}

/** 
 * Checks whether the cron file was modified and if so, reloads it.
 * */
tasklist_t update_tasklist(tasklist_t tasklist, char* filename, time_t* last_mtime)
{
  time_t mtime = get_mtime(filename);
  if(mtime > *last_mtime)
  {
    *last_mtime = mtime;
    tasklist_clear(tasklist);
    return loadFromFile(filename);
  }
  return tasklist; 
}

/**
 * Copies output of a task into output of cron
 * */
void catpid(int status, int pid)
{
  char* name = get_tmp_name(getpid(), pid);
  int fd = open(name, O_RDONLY);
  if(!fd) stderror(STD_ERR);
  char buff[BUFFSIZE];
  int bytes_read;
  while((bytes_read = read(fd, buff, BUFFSIZE)) > 0)
    write(STD_OUT, buff, bytes_read);
  char* msg = "process exitted with status ";
  char* spid = itoa(status);
  char* nl = "\n";
  write(STD_OUT, msg, strlen(msg));
  write(STD_OUT, spid, strlen(spid));
  write(STD_OUT, nl, strlen(nl));
  write(STD_OUT, nl, strlen(nl));
  write(STD_OUT, nl, strlen(nl));
  free(spid);
  free(name);
}

/**
 * This function cleans finished threads. I.e. Goes through all finished tasks and initiates processing of their output.
 * */
void clean(bool bwait)
{
  int status;
  int flags = bwait ? 0 : WNOHANG;
  while(true)
  {
    int pid = waitpid(0, &status, flags);
    if(pid <= 0)
      break;
    catpid(status, pid);
  }
}

/**
 * This is the main event loop of the daemon
 * */
void run_daemon(char* filename, bool once)
{
  time_t last_mtime = get_mtime(filename);
  tasklist_t tasklist = loadFromFile(filename);
  time_t now = timestamp();
  while(true)
  {
    time_t next = now + 60 - now%60;
    tasklist = update_tasklist(tasklist, filename, &last_mtime);
    execute_tasks(tasklist, now);
    sleep(1); //for immediate output of tasks finishing under 1 second - just for tester - friendliness, longre running tasks will be outputted later
    clean(false);
    if(once)
    {
      //printf("> waiting for processes to exit");
      clean(true);
      exit(0);
    }
    time_t then = timestamp();
    if(then > next)
      error("warning: forking sheduled processes took over one minute! We are behind schedule!\n", false, STD_ERR);
    else
      sleep(next-then);
    now = next;
  }
}


/**
 * Detaches the process from console.
 * */
void run_daemonized(char* filename, bool once)
{
  int pid = fork();
  if(pid < 0)
  {
    stderror(STD_ERR);
  }
  if(pid == 0)
  {
    setsid();
    run_daemon(filename, once);
    exit(0);
  }
  else
  {
    exit(0);
  }
}

int main(int argc, char * const argv[])
{
  int opt;
  char* filename = "";
  bool daemonized = false;
  bool once = false;
  while((opt = getopt(argc, argv, "sdof:")) != -1)
    switch(opt) {
      case 'o':
        once = true;
        break;
      case 'f':
        filename = optarg; 
        break;
      case 'd':
        daemonized = true;
        break;
      case 's':
        close(0);
        close(1);
        close(2);
        break;
      default: 
        help();
        exit(1);
    }

  if(*filename == '\0')
  {
    error("nonempty filename must be given!\n", false, STD_ERR);
    help();
    exit(1);
  }

  if(daemonized)
    run_daemonized(filename, once);
  else
    run_daemon(filename, once);

  return 0;
}
