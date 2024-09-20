#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>

#include <dlfcn.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "ht_server.h"
#include "ht_table.h"
#include "ht_log.h"

#define sock_write_no_warn(fd, msg, len) ((void)(write((fd), (msg), (len)) + 1))

static const char *human_readable_size(size_t size)
{
	static char buf[128];

	if (size < 10 * 1024) {
		sprintf(buf, "%zu bytes", size);
	} else if (size < 3 * 1024 * 1024) {
		sprintf(buf, "%zu Kb (%zu bytes)", size / 1024, size);
	} else if (size < 1024 * 1024 * 1024) {
		sprintf(buf, "%zu Mb (%zu Kb)", size / 1024 / 1024,
			size / 1024);
	} else {
		printf(buf, "%zu Gb (%zu mb)", size / 1024 / 1024 / 1024,
		       size / 1024 / 1024);
	}

	return buf;
}

static void send_alloc_stat(const ht_alloc_stat_t *stat, void *arg)
{
	static char stat_buf[1024];
	char *ptr = stat_buf;

	for (size_t i = 0; i < stat->bt.size; ++i) {
		void *bt = stat->bt.entries[i];

		Dl_info info;
		if (dladdr(bt, &info) == 0) {
			ptr += sprintf(ptr, "%p (unknown)\n", bt);
		} else {
			void *calibrated = (void *)((uintptr_t)bt -
						    (uintptr_t)info.dli_fbase);

			ptr += sprintf(ptr, "%p (%p) | %s of %s\n", calibrated,
				       bt, info.dli_sname, info.dli_fname);
		}
	}

	ptr += sprintf(ptr, "\tallocs = %llu free = %llu total_size = %s\n\n",
		       stat->alloc_count, stat->free_count,
		       human_readable_size(stat->total_size));

	*ptr = 0;

	int *clientfd = arg;
	sock_write_no_warn(*clientfd, stat_buf, strlen(stat_buf));
}

static int server_routine(__attribute__((unused)) void *arg)
{
	char path[128];
	sprintf(path, "/tmp/ht.%d.sock", getpid());

	int fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (fd == -1) {
		ht_log_perror("socket");
		abort();
	}

	struct sockaddr_un addr;
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, path);
	unlink(path);

	if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		ht_log_perror("bind");
		abort();
	}

	listen(fd, 10);

	while (1) {
		int clientfd = accept(fd, NULL, NULL);
		if (clientfd == -1) {
			ht_log_perror("accept");
			abort();
		}

		size_t total_allocs, used_buckets;

		ht_table_foreach_stat(send_alloc_stat, &clientfd, &total_allocs,
				      &used_buckets);

		char buf[128];
		sprintf(buf, "allocations = %zu buckets = %zu\n", total_allocs,
			used_buckets);
		sock_write_no_warn(clientfd, buf, strlen(buf));

		close(clientfd);
	}

	return 0;
}

void ht_start_server()
{
	static char server_stack[16384];

	int clonerc =
		clone(server_routine, server_stack + sizeof(server_stack),
		      CLONE_THREAD | CLONE_FILES | CLONE_VM | CLONE_SIGHAND,
		      NULL);
	if (clonerc == -1) {
		ht_log_perror("clone");
	}
}