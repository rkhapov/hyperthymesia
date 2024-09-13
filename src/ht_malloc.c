#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "ht_alloc_header.h"
#include "ht_bt.h"
#include "ht_table.h"
#include "ht_malloc.h"

void *ht_malloc(size_t size)
{
	const size_t total_size = sizeof(ht_alloc_header_t) + size;

	void *raw = malloc(total_size);
	if (raw == NULL) {
		return NULL;
	}

	ht_alloc_header_t *header = raw;

	header->alloc_size = size;

	if (ht_bt_collect(&header->alloc_bt)) {
		free(raw);
		return NULL;
	}

	ht_alloc_stat_t *stats =
		ht_table_get_allocation_stats(&header->alloc_bt);
	if (stats == NULL) {
		free(raw);
		return NULL;
	}

	stats->alloc_count++;
	stats->total_size += size;

	return (void *)(header + 1);
}

void ht_free(void *ptr)
{
	ht_alloc_header_t *header = ptr;

	header--;

	ht_alloc_stat_t *stats =
		ht_table_get_allocation_stats(&header->alloc_bt);
	assert(stats != NULL);

	stats->free_count++;
	stats->total_size -= header->alloc_size;

	free((void *)header);
}

void *ht_realloc(void *ptr, size_t size)
{
	ht_alloc_header_t *header = ptr;
	header--;

	size_t old_size = header->alloc_size;


	void *newptr = realloc(ptr, size);
}