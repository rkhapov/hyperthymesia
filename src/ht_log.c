#include <unistd.h>
#include <string.h>

#include "ht_log.h"

void ht_log_stderr(const char *msg)
{
	write(1, msg, strlen(msg));
	write(1, "\n", 1);
}
