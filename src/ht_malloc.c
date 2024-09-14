#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "ht_alloc_header.h"
#include "ht_bt.h"
#include "ht_table.h"
#include "ht_malloc.h"

static void register_allocation(ht_alloc_stat_t *stats, size_t size)
{
	stats->total_size += size;
	stats->alloc_count++;
}

static void register_deallocation(ht_alloc_stat_t *stats, size_t size)
{
	stats->total_size -= size;
	stats->free_count++;
}

void *ht_malloc(size_t size)
{
	const size_t total_size = sizeof(ht_alloc_header_t) + size;

	void *raw = malloc(total_size);
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

	ht_alloc_stat_t *stats =
		ht_table_get_allocation_stats(&header->alloc_bt);
	if (stats == NULL) {
		// TODO: do smth more supportable here ?
		perror("can't get stats at malloc");
		abort();
	}

	register_allocation(stats, size);

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
	// realloc-compatibility
	if (ptr == NULL) {
		return ht_malloc(size);
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

	ht_alloc_header_t *header = ptr;
	header--;

	header = realloc(header, sizeof(ht_alloc_header_t) + size);
	if (header == NULL) {
		return NULL;
	}

	ht_alloc_stat_t *stats =
		ht_table_get_allocation_stats(&header->alloc_bt);
	if (stats == NULL) {
		// TODO: do smth more supportable here ?
		perror("can't get stats at realloc");
		abort();
	}

	register_deallocation(stats, header->alloc_size);

	header->alloc_size = size;
	if (ht_bt_collect(&header->alloc_bt, 1)) {
		// extremely unlikely...
		// TODO: do smth more supportable here ?
		perror("can't get backtrace at realloc");
		abort();
	}

	// if it is the same bt
	// (when realloc called twice at same scenario)
	// there is no need to traverse stat table
	if (!ht_bt_equals(&stats->bt, &header->alloc_bt)) {
		stats = ht_table_get_allocation_stats(&header->alloc_bt);
		if (stats == NULL) {
			// TODO: do smth more supportable here ?
			perror("can't get stats at realloc");
			abort();
		}
	}

	register_allocation(stats, size);

	return (void *)(header + 1);
}