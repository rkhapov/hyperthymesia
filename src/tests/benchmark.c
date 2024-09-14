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

static int cmp_double(const void *a, const void *b)
{
	double aa = *((const double *)a);
	double bb = *((const double *)b);

	if (aa < bb) {
		return -1;
	}

	if (aa > bb) {
		return 1;
	}

	return 0;
}

static double test_get_malloc_ratio_ht_to_pure(const int alloc_count,
					       const int alloc_size,
					       uint64_t **ptrs)
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
			perror("malloc");
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

static double test_get_ralloc_ratio_ht_to_pure(const int alloc_count,
					       const int alloc_size,
					       const int growth,
					       uint64_t **ptrs)
{
	for (int i = 0; i < alloc_count; ++i) {
		ptrs[i] = (uint64_t *)ht_malloc(alloc_size);
		if (ptrs[i] == NULL) {
			perror("ht_malloc");
			abort();
		}
	}

	unsigned long long ht_time_start = get_time_ns();
	for (int i = 0; i < alloc_count; ++i) {
		ptrs[i] = (uint64_t *)ht_realloc(ptrs[i], alloc_size + growth);
		if (ptrs[i] == NULL) {
			perror("ht_realloc");
			abort();
		}
	}
	unsigned long long ht_time_end = get_time_ns();

	for (int i = 0; i < alloc_count; ++i) {
		ht_free(ptrs[i]);
	}

	for (int i = 0; i < alloc_count; ++i) {
		ptrs[i] = (uint64_t *)malloc(alloc_size);
		if (ptrs[i] == NULL) {
			perror("malloc");
			abort();
		}
	}

	unsigned long long pure_time_start = get_time_ns();
	for (int i = 0; i < alloc_count; ++i) {
		ptrs[i] = (uint64_t *)realloc(ptrs[i], alloc_size + growth);
		if (ptrs[i] == NULL) {
			perror("realloc");
			abort();
		}
	}
	unsigned long long pure_time_end = get_time_ns();

	for (int i = 0; i < alloc_count; ++i) {
		free((void *)ptrs[i]);
	}

	unsigned long long ht_time_total = (ht_time_end - ht_time_start);
	unsigned long long pure_time_total = (pure_time_end - pure_time_start);

	return (double)ht_time_total / (double)pure_time_total;
}

#define benchmark_alloc_count 100000
static uint64_t *benchmark_ptrs[benchmark_alloc_count];

static const int alloc_sizes[] = { 1 << 3, 1 << 4,  1 << 8,
				   1 << 9, 1 << 12, 1 << 16 };

void test_malloc_benchmark()
{
	ht_thread_table_init(49999, 30);

	for (unsigned i = 0; i < sizeof(alloc_sizes) / sizeof(int); ++i) {
		const int measurements_count = 30;
		double measurements[measurements_count];
		const int alloc_size = alloc_sizes[i];

		for (int i = 0; i < measurements_count; ++i) {
			double ratio = test_get_malloc_ratio_ht_to_pure(
				benchmark_alloc_count, alloc_size,
				benchmark_ptrs);

			measurements[i] = ratio;
		}

		qsort(measurements, measurements_count, sizeof(double),
		      cmp_double);
		double min = measurements[1];
		double max = measurements[measurements_count - 2];

		printf("[benchmark] allocations of size %d is slower on ht from %f to %f times\n",
		       alloc_size, min, max);
	}

	ht_thread_table_destroy();
}

void test_realloc_benchmark()
{
	ht_thread_table_init(49999, 30);

	for (unsigned i = 0; i < sizeof(alloc_sizes) / sizeof(int); ++i) {
		const int measurements_count = 30;
		double measurements[measurements_count];
		const int alloc_size = alloc_sizes[i];
		const int growth = (int)(alloc_size * 0.2L);

		for (int i = 0; i < measurements_count; ++i) {
			double ratio = test_get_ralloc_ratio_ht_to_pure(
				benchmark_alloc_count, alloc_size, growth,
				benchmark_ptrs);

			measurements[i] = ratio;
		}

		qsort(measurements, measurements_count, sizeof(double),
		      cmp_double);
		double min = measurements[1];
		double max = measurements[measurements_count - 2];

		printf("[benchmark] reallocations from %d to %d is slower on ht from %f to %f times\n",
		       alloc_size, alloc_size + growth, min, max);
	}

	ht_thread_table_destroy();
}
