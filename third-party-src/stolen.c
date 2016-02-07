#include "stolen.h"
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include "../src/common.h"

//those are 'stolen' from stack overflow

char * itoa(int i) 
{
  int n = snprintf(NULL, 0, "%d", i) + 1;
  char *s = malloc(n); CHECKP(s, "alloc error") 

  if (s != NULL)
    snprintf(s, n, "%d", i);
  return s;
}

char* concat(char *s1, char *s2) 
{
  char *result = malloc(strlen(s1)+strlen(s2)+1); CHECKP(result, "alloc error")
  strcpy(result, s1);
  strcat(result, s2);
  return result;
}

