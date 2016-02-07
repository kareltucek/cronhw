#include "common.h"
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

void
stderror(int fd)
{
	error(strerror(errno), false, fd);
}

static bool silent;

void
set_silent(bool s)
{
	silent = s;
}

bool
get_silent()
{
	return (silent);
}

void
error(char *err, bool critical, int fd)
{
	if (write(fd, err, strlen(err)) < 0)
		exit(1);
	if (critical)
		exit(1);
}

