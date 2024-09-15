#ifndef HYPERTHYMESIA_REAL_FUNC_H
#define HYPERTHYMESIA_REAL_FUNC_H

#include <stddef.h>

typedef void *(*ht_malloc_func_t)(size_t);
typedef void *(*ht_realloc_func_t)(void *, size_t);
typedef void (*ht_free_func_t)();

ht_malloc_func_t ht_get_real_malloc();
ht_realloc_func_t ht_get_real_realloc();
ht_free_func_t ht_get_real_free();

#endif // HYPERTHYMESIA_REAL_FUNC_H
