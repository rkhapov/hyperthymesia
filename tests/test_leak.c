#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <dlfcn.h>

#include <unistd.h>

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
	do_register_stack_hint_func();

	int leaked = 0;
	int prev_leaked = 0;
	while (1) {
		void *ptr = t1_func1();
		if ((rand() % 100) > 40) {
			free(ptr);
		} else {
			leaked += 100;
		}

		if (leaked != prev_leaked) {
			printf("[t1] leaked %d\n", leaked);
			prev_leaked = leaked;
		}

		sleep(1);
	}

	return NULL;
}

NO_INLINE void *t2_realloc_func2(void *ptr)
{
	return realloc(ptr, 200);
}

NO_INLINE void *t2_realloc_func1(void *ptr)
{
	return t2_realloc_func2(ptr);
}

NO_INLINE void *t2_func2()
{
	return malloc(100);
}

NO_INLINE void *t2_func1()
{
	return t2_func2();
}

void *thread2_routine(UNUSED void *arg)
{
	do_register_stack_hint_func();

	int leaked = 0;
	int prev_leaked = 0;
	while (1) {
		void *ptr = t2_func1();
		if ((rand() & 1) == 1) {
			free(ptr);
		} else {
			ptr = t2_realloc_func1(ptr);
			if ((rand() & 1) == 1) {
				free(ptr);
			} else {
				leaked += 200;
			}
		}

		if (leaked != prev_leaked) {
			printf("[t2] leaked %d\n", leaked);
			prev_leaked = leaked;
		}

		sleep(1);
	}

	return NULL;
}

NO_INLINE void *t3_func2()
{
	return calloc(10, 10);
}

NO_INLINE void *t3_func1()
{
	return t3_func2();
}

void *thread3_routine(UNUSED void *arg)
{
	do_register_stack_hint_func();

	int leaked = 0;
	int prev_leaked = 0;
	while (1) {
		void *ptr = t2_func1();
		if ((rand() & 1) == 1) {
			free(ptr);
		} else {
			leaked += 100;
		}

		if (leaked != prev_leaked) {
			printf("[t3] leaked %d\n", leaked);
			prev_leaked = leaked;
		}

		sleep(1);
	}

	return NULL;
}

int main()
{
	do_register_stack_hint_func();

	srand(time(NULL));

	printf("my pid is %d\n", getpid());

	pthread_t thread1;
	pthread_t thread2;
	pthread_t thread3;

	pthread_attr_t attr;
	pthread_attr_init(&attr);

	pthread_create(&thread1, &attr, thread1_routine, NULL);
	pthread_create(&thread2, &attr, thread2_routine, NULL);
	pthread_create(&thread3, &attr, thread3_routine, NULL);

	pthread_join(thread1, NULL);
	pthread_join(thread2, NULL);
	pthread_join(thread3, NULL);

	return 0;
}
