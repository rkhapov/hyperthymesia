#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "ht_log.h"

void ht_log_perror(const char *msg)
{
	write(1, msg, strlen(msg));
	write(1, ": ", 2);
	char *err = strerror(errno);
	write(1, err, strlen(err));
	write(1, "\n", 1);
}

void ht_log_stderr(const char *msg)
{
	write(1, msg, strlen(msg));
	write(1, "\n", 1);
}
