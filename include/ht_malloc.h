#ifndef HYPERTHYMESIA_MALLOC_H
#define HYPERTHYMESIA_MALLOC_H

#include <stddef.h>

void *malloc(size_t size);
void free(void *ptr);
void *realloc(void *ptr, size_t size);

#endif // HYPERTHYMESIA_MALLOC_H
