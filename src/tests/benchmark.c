#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "ht_malloc.h"
#include "ht_table.h"

static unsigned long long get_time_ns()
{
	struct timespec ts;
	if (clock_gettime(CLOCK_MONOTONIC_RAW, &ts) < 0) {
		perror("can't get time ns");
		abort();
	}

	return 1000000000ULL * ts.tv_sec + ts.tv_nsec;
}

static double test_get_ratio_ht_to_pure(const int alloc_count,
					const int alloc_size, uint64_t **ptrs)
{
	unsigned long long ht_time_start = get_time_ns();
	for (int i = 0; i < alloc_count; ++i) {
		ptrs[i] = (uint64_t *)ht_malloc(alloc_size);
		if (ptrs[i] == NULL) {
			perror("ht_malloc");
			abort();
		}
	}

	for (int i = 0; i < alloc_count; ++i) {
		ht_free(ptrs[i]);
	}
	unsigned long long ht_time_end = get_time_ns();

	unsigned long long pure_time_start = get_time_ns();
	for (int i = 0; i < alloc_count; ++i) {
		ptrs[i] = (uint64_t *)malloc(alloc_size);
		if (ptrs[i] == NULL) {
			perror("ht_malloc");
			abort();
		}
	}

	for (int i = 0; i < alloc_count; ++i) {
		free((void *)ptrs[i]);
	}
	unsigned long long pure_time_end = get_time_ns();

	unsigned long long ht_time_total = (ht_time_end - ht_time_start);
	unsigned long long pure_time_total = (pure_time_end - pure_time_start);

	return (double)ht_time_total / (double)pure_time_total;
}

#define benchmark_alloc_count 100000
static uint64_t *benchmark_ptrs[benchmark_alloc_count];

void test_malloc_benchmark()
{
	ht_thread_table_init((1 << 16), 30);

	const int alloc_sizes[] = { 1 << 3, 1 << 4,  1 << 8,
				    1 << 9, 1 << 12, 1 << 16 };

	for (unsigned i = 0; i < sizeof(alloc_sizes) / sizeof(int); ++i) {
		double min = INFINITY;
		double max = 0.0;
		const int alloc_size = alloc_sizes[i];

		for (int i = 0; i < 30; ++i) {
			double ratio =
				test_get_ratio_ht_to_pure(benchmark_alloc_count,
							  alloc_size,
							  benchmark_ptrs);

			if (ratio > max) {
				max = ratio;
			}

			if (ratio < min) {
				min = ratio;
			}
		}

		printf("[benchmark] allocations of size %d is slower on ht from %f to %f times\n",
		       alloc_size, min, max);
	}

	ht_thread_table_destroy();
}
