#include <stdlib.h>
#include <pthread.h>

#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "ht_server.h"
#include "ht_log.h"

static pthread_t server_thread_id;
static pthread_attr_t server_thread_attr;

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

		write(clientfd, "hello!", 6);
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