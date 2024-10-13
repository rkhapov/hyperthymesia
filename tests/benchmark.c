#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <time.h>
#include <pthread.h>

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <dlfcn.h>

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

#define benchmark_alloc_count 100000
static uint64_t *benchmark_ptrs[benchmark_alloc_count];
static const int alloc_sizes[] = { 1 << 3, 1 << 4,  1 << 8,
				   1 << 9, 1 << 12, 1 << 14 };

static unsigned long long get_time_ns()
{
	struct timespec ts;
	if (clock_gettime(CLOCK_MONOTONIC_RAW, &ts) < 0) {
		perror("can't get time ns");
		abort();
	}

	return 1000000000ULL * ts.tv_sec + ts.tv_nsec;
}

static void run_malloc_test()
{
	for (size_t i = 0; i < sizeof(alloc_sizes) / sizeof(int); ++i) {
		const int alloc_size = alloc_sizes[i];

		unsigned long long start_time = get_time_ns();
		for (int i = 0; i < benchmark_alloc_count; ++i) {
			benchmark_ptrs[i] = malloc(alloc_size);
			if (benchmark_ptrs[i] == NULL) {
				abort();
			}
		}
		unsigned long long total_time = get_time_ns() - start_time;

		for (int i = 0; i < benchmark_alloc_count; ++i) {
			memset(benchmark_ptrs[i], 'a', alloc_size);
		}

		for (int i = 0; i < benchmark_alloc_count; ++i) {
			free(benchmark_ptrs[i]);
		}

		printf("%d %llu\n", alloc_size, total_time);
	}
}

static void run_realloc_test()
{
	for (size_t i = 0; i < sizeof(alloc_sizes) / sizeof(int); ++i) {
		const int alloc_size = alloc_sizes[i];

		for (int i = 0; i < benchmark_alloc_count; ++i) {
			benchmark_ptrs[i] = malloc(alloc_size);
			if (benchmark_ptrs[i] == NULL) {
				abort();
			}

			memset(benchmark_ptrs[i], 'a', alloc_size);
		}

		int new_size = (int)((double)alloc_size * 1.5);

		unsigned long long start_time = get_time_ns();
		for (int i = 0; i < benchmark_alloc_count; ++i) {
			benchmark_ptrs[i] =
				realloc(benchmark_ptrs[i], new_size);
			if (benchmark_ptrs[i] == NULL) {
				abort();
			}
		}
		unsigned long long total_time = get_time_ns() - start_time;

		for (int i = 0; i < benchmark_alloc_count; ++i) {
			memset(benchmark_ptrs[i], 'a', new_size);
		}

		for (int i = 0; i < benchmark_alloc_count; ++i) {
			free(benchmark_ptrs[i]);
		}

		printf("%d %llu\n", alloc_size, total_time);
	}
}

int main(int argc, char **argv)
{
	do_register_stack_hint_func();

	if (argc < 2) {
		printf("expected argument: function name (malloc or realloc) to test\n");
		return 1;
	}

	if (strcmp(argv[1], "malloc") == 0) {
		run_malloc_test();
	} else if (strcmp(argv[1], "realloc") == 0) {
		run_realloc_test();
	} else {
		printf("unknown mode %s\n", argv[1]);
		return 1;
	}

	return 0;
}
