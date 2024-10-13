#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <unistd.h>
#include <dlfcn.h>

#define NO_INLINE __attribute__((noinline))
#define UNUSED __attribute__((unused))

__thread void *stack_begin = NULL;
__thread void *stack_end = NULL;

void ht_stack_location_hint(void **begin, void **end)
{
	if (__builtin_expect(stack_begin == NULL || stack_end == NULL, 0)) {
		pthread_attr_t attr;
		void *stackaddr;
		size_t stacksize;

		pthread_getattr_np(pthread_self(), &attr);
		pthread_attr_getstack(&attr, &stackaddr, &stacksize);

		stack_begin = stackaddr;
		stack_end = (void *)((uintptr_t)stackaddr + stacksize);
	}

	*begin = stack_begin;
	*end = stack_end;

	return;
}

typedef void (*stack_hintfn_t)(void **begin, void **end);
void do_register_stack_hint_func()
{
	void (*registerfn)(stack_hintfn_t);
	*(void **)(&registerfn) =
		dlsym(RTLD_DEFAULT, "ht_register_stack_location_hint");
	if (registerfn == NULL) {
		return;
	}

	registerfn(ht_stack_location_hint);
}

NO_INLINE void *p1_2()
{
	return malloc(100);
}

NO_INLINE void *p1_1()
{
	return p1_2();
}

NO_INLINE void parent_routine()
{
	int leaked = 0;
	int prev_leaked = 0;
	while (1) {
		void *ptr = p1_1();
		if ((rand() & 1) == 1) {
			free(ptr);
		} else {
			leaked += 100;
		}

		if (leaked != prev_leaked) {
			printf("[parent] leaked %d\n", leaked);
			prev_leaked = leaked;
		}

		sleep(1);
	}
}

NO_INLINE void *p2_2()
{
	return malloc(500);
}

NO_INLINE void *p2_1()
{
	return p2_2();
}

NO_INLINE void child_routine()
{
	int leaked = 0;
	int prev_leaked = 0;
	while (1) {
		void *ptr = p2_1();
		if ((rand() & 1) == 1) {
			free(ptr);
		} else {
			leaked += 500;
		}

		if (leaked != prev_leaked) {
			printf("[child] leaked %d\n", leaked);
			prev_leaked = leaked;
		}

		sleep(1);
	}
}

int main()
{
	do_register_stack_hint_func();

	srand(time(NULL));

	printf("parent pid is %d\n", getpid());

	pid_t rf = fork();
	if (rf == -1) {
		abort();
	} else if (rf == 0) {
		srand(time(NULL) + 100);
		child_routine();
	} else {
		printf("child pid is %d\n", rf);
		parent_routine();
	}

	return 0;
}