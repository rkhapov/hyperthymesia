#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include <dlfcn.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "ht_server.h"
#include "ht_table.h"
#include "ht_log.h"

#define sock_write_no_warn(fd, msg, len) ((void)(write((fd), (msg), (len)) + 1))

static pthread_t server_thread_id;
static pthread_attr_t server_thread_attr;

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

			ptr += sprintf(ptr, "addr2line -e %s %p | %s\n",
				       info.dli_fname, calibrated,
				       info.dli_sname);
		}
	}

	ptr += sprintf(ptr, "\tallocs = %llu free = %llu total_size = %s\n\n",
		       stat->alloc_count, stat->free_count,
		       human_readable_size(stat->total_size));

	*ptr = 0;

	int *clientfd = arg;
	sock_write_no_warn(*clientfd, stat_buf, strlen(stat_buf));
}

static void *server_routine(void *arg)
{
	const char *path = (const char *)arg;

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

		ht_table_foreach_stat(send_alloc_stat, &clientfd);

		close(clientfd);
	}
}

void ht_start_server(const char *socket_path)
{
	if (pthread_attr_init(&server_thread_attr)) {
		ht_log_perror("pthread_attr_init");
		abort();
	}

	if (pthread_create(&server_thread_id, &server_thread_attr,
			   server_routine, (void *)socket_path)) {
		ht_log_perror("pthread_create");
		abort();
	}

	pthread_detach(server_thread_id);
}