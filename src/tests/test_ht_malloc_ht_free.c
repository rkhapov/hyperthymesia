#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <unistd.h>
#include <sys/resource.h>

#include "ht_defer.h"
#include "ht_table.h"
#include "ht_malloc.h"

static size_t get_process_rss()
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
	ht_thread_table_init(49999, 30);

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

	ht_thread_table_destroy();
}
