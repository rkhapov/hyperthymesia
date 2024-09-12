#include <stdlib.h>

#include "ht_malloc.h"

void *ht_malloc(size_t size)
{
	return malloc(size);
}

void ht_free(void *ptr)
{
	free(ptr);
}

void *ht_realloc(void *ptr, size_t size)
{
	return realloc(ptr, size);
}