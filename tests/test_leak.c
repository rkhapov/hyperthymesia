#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>

#include <unistd.h>

#define NO_INLINE __attribute__((noinline))
#define UNUSED __attribute__((unused))

NO_INLINE void *t1_func2()
{
	return malloc(100);
}

NO_INLINE void *t1_func1()
{
	return t1_func2();
}

void *thread1_routine(UNUSED void *arg)
{
	int leaked = 0;
	while (1) {
		void *ptr = t1_func1();
		if ((rand() % 100) > 40) {
			free(ptr);
		} else {
			leaked += 100;
		}

		printf("[t1] leaked %d\n", leaked);

		sleep(1);
	}

	return NULL;
}

void *thread2_routine(UNUSED void *arg)
{
	while (1) {
		sleep(1);
	}

	return NULL;
}

int main()
{
	srand(time(NULL));

	pthread_t thread1;
	pthread_t thread2;

	pthread_attr_t attr;
	pthread_attr_init(&attr);

	pthread_create(&thread1, &attr, thread1_routine, NULL);
	pthread_create(&thread2, &attr, thread2_routine, NULL);

	pthread_join(thread1, NULL);
	pthread_join(thread2, NULL);

	return 0;
}