#include <stdlib.h>
#include <string.h>

#include "ht_alloc_header.h"
#include "ht_malloc.h"

void *ht_malloc(size_t size)
{
	const size_t total_size = size + sizeof(ht_alloc_header_t);

	void *raw = malloc(total_size);
	if (raw == NULL) {
		return NULL;
	}

	ht_alloc_header_t *header = raw;

	// TODO: set header->alloc_bt here
	// TODO: put allocation into allocations table

	return (void *)(header + 1);
}

void ht_free(void *ptr)
{
	ht_alloc_header_t *header = ptr;

	header--;

	free((void *)header);
}

void *ht_realloc(void *ptr, size_t size)
{
	return realloc(ptr, size);
}