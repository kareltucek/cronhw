#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>

void stderror(int fd)
{
  error(strerror(errno), false, fd);
}

void error(char* err, bool critical, int fd)
{
  write(fd, err, strlen(err));
  if(critical)
    exit(1);
}

