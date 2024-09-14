#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ht_table.h"
#include "ht_malloc.h"

void test_ht_realloc()
{
	ht_thread_table_init(49999, 30);

	const int alloc_count = 100;
	const int alloc_size = 1024;

	void *ptrs[alloc_count];
	for (int i = 0; i < alloc_count; ++i) {
		ptrs[i] = ht_malloc(alloc_size);
		if (ptrs[i] == NULL) {
			perror("ht_malloc");
			abort();
		}

		memset(ptrs[i], 'a', alloc_size);
	}

	for (int i = 0; i < alloc_count; ++i) {
		ptrs[i] = ht_realloc(ptrs[i], alloc_size * 2);
		if (ptrs[i] == NULL) {
			perror("ht_realloc");
			abort();
		}

		memset(ptrs[i], 'b', alloc_size * 2);
	}

	for (int i = 0; i < alloc_count; ++i) {
		ht_free(ptrs[i]);
	}

	ht_thread_table_destroy();
}