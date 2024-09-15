#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <unistd.h>

#include "ht_log.h"

#define log_write(a, b, c) ((void)(write(a, b, c) + 1))

void ht_log_perror(const char *msg)
{
	log_write(1, msg, strlen(msg));
	log_write(1, ": ", 2);
	char *err = strerror(errno);
	log_write(1, err, strlen(err));
	log_write(1, "\n", 1);
}

void ht_log_stderr(const char *msg)
{
	log_write(1, msg, strlen(msg));
	log_write(1, "\n", 1);
}
