#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <math.h>

#include <unistd.h>
#include <sys/resource.h>

#include "ht_malloc.h"
#include "ht_defer.h"

size_t get_process_rss()
{
	long rss = 0L;
	FILE *fp = NULL;
	ht_defer(ht_close_file, &fp);
	if ((fp = fopen("/proc/self/statm", "r")) == NULL) {
		perror("can't open /proc/self/statm");
		abort();
	}

	if (fscanf(fp, "%*s%ld", &rss) != 1) {
		perror("can't read /proc/self/statm");
		abort();
	}

	return (size_t)rss * (size_t)sysconf(_SC_PAGESIZE);
}

void test_ht_malloc_ht_free()
{
	const int ptrs_count = 100;
	const size_t alloc_size = 10 * 1024 * 1024;

	char *ptrs[ptrs_count];

	size_t rss_start = get_process_rss();
	printf("[ht_malloc & ht_free] before allocation rss = %zu\n",
	       rss_start);

	for (int i = 0; i < ptrs_count; ++i) {
		char *mem = ht_malloc(alloc_size);
		if (mem == NULL) {
			abort();
		}

		// check is mem are available for writing
		memset(mem, 42, alloc_size);

		ptrs[i] = mem;
	}

	size_t rss_after_alloc = get_process_rss();
	printf("[ht_malloc & ht_free] after allocation rss = %zu\n",
	       rss_after_alloc);

	size_t growth = rss_after_alloc - rss_start;

	if (growth < ptrs_count * alloc_size) {
		printf("[ht_malloc & ht_free] rss growth is less than allocated mem\n");
		abort();
	}

	for (int i = 0; i < ptrs_count; ++i) {
		ht_free(ptrs[i]);
	}

	size_t rss_after_free = get_process_rss();
	printf("[ht_malloc & ht_free] after free rss = %zu\n", rss_after_free);
	if (rss_after_free >= rss_after_alloc) {
		printf("[ht_malloc & ht_free] ht_free doesn't work\n");
		abort();
	}
}

unsigned long long get_time_ns()
{
	struct timespec ts;
	if (clock_gettime(CLOCK_MONOTONIC_RAW, &ts) < 0) {
		perror("can't get time ns");
		abort();
	}

	return 1000000000ULL * ts.tv_sec + ts.tv_nsec;
}

double test_get_ratio_ht_to_pure(const int alloc_count, const int alloc_size,
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

		printf("[benchmark] allocation size %d is slower on ht from %f to %f times\n",
		       alloc_size, min, max);
	}
}

int main(int argc, char **argv)
{
	test_ht_malloc_ht_free();

	test_malloc_benchmark();

	return 0;
}
