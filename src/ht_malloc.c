#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <malloc.h>

#include "ht_alloc_header.h"
#include "ht_bt.h"
#include "ht_table.h"
#include "ht_real_funcs.h"
#include "ht_malloc.h"

void *malloc(size_t size)
{
	ht_malloc_func_t real_malloc = ht_get_real_malloc();

	const size_t total_size = sizeof(ht_alloc_header_t) + size;
	void *raw = real_malloc(total_size);
	if (raw == NULL) {
		return NULL;
	}

	ht_alloc_header_t *header = raw;

	if (ht_bt_collect(&header->alloc_bt, 1)) {
		// TODO: do smth more supportable here ?
		perror("can't get backtrace at malloc");
		abort();
	}

	header->alloc_size = size;

	ht_table_register_allocation(&header->alloc_bt, size);

	return (void *)(header + 1);
}

void free(void *ptr)
{
	ht_free_func_t real_free = ht_get_real_free();

	ht_alloc_header_t *header = ptr;
	header--;

	ht_table_register_deallocation(&header->alloc_bt, header->alloc_size);

	real_free((void *)header);
}

void *realloc(void *ptr, size_t size)
{
	// realloc-compatibility
	if (ptr == NULL) {
		return malloc(size);
	}

	/*
	 * For realloc we must perform two statistics registations:
	 * - 'deallocation' of the old memory
	 * - 'allocation' of the new one
	 * even if memory region is enlarged and not moved.
	 * It is necessary because its more clear way to track
	 * realloc as new allocation, instead of 'seeing' growing
	 * some allocated memory, with wrong allocation backtrace.
	*/

	ht_realloc_func_t real_realloc = ht_get_real_realloc();

	ht_alloc_header_t *header = ptr;
	header--;

	header = real_realloc(header, sizeof(ht_alloc_header_t) + size);
	if (header == NULL) {
		return NULL;
	}

	ht_table_register_deallocation(&header->alloc_bt, header->alloc_size);

	header->alloc_size = size;
	if (ht_bt_collect(&header->alloc_bt, 1)) {
		// extremely unlikely...
		// TODO: do smth more supportable here ?
		perror("can't get backtrace at realloc");
		abort();
	}

	ht_table_register_allocation(&header->alloc_bt, size);

	return (void *)(header + 1);
}