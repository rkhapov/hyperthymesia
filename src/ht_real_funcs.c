#include <stdlib.h>
#include <dlfcn.h>

#include "ht_real_funcs.h"

#define unlikely(x) __builtin_expect(!!(x), 0)

__thread ht_malloc_func_t real_malloc = NULL;
__thread ht_realloc_func_t real_realloc = NULL;
__thread ht_free_func_t real_free = NULL;

ht_malloc_func_t ht_get_real_malloc()
{
	if (unlikely(real_malloc == NULL)) {
		real_malloc = (ht_malloc_func_t)dlsym(RTLD_NEXT, "malloc");
		if (real_malloc == NULL) {
			abort();
		}
	}

	return real_malloc;
}

ht_realloc_func_t ht_get_real_realloc()
{
	if (unlikely(real_realloc == NULL)) {
		real_realloc = (ht_realloc_func_t)dlsym(RTLD_NEXT, "realloc");
		if (real_malloc == NULL) {
			abort();
		}
	}

	return real_realloc;
}

ht_free_func_t ht_get_real_free()
{
	if (unlikely(real_free == NULL)) {
		real_free = (ht_free_func_t)dlsym(RTLD_NEXT, "free");
		if (real_free == NULL) {
			abort();
		}
	}

	return real_free;
}
