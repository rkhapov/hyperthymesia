#ifndef HYPERTHYMESIA_MALLOC_H
#define HYPERTHYMESIA_MALLOC_H

#include <stddef.h>

void *ht_malloc(size_t size);
void ht_free(void *ptr);
void *ht_realloc(void *ptr, size_t size);

#endif // HYPERTHYMESIA_MALLOC_H
