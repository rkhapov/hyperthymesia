#include <stdlib.h>
#include <dlfcn.h>

#include "ht_real_funcs.h"

#define unlikely(x) __builtin_expect(!!(x), 0)

__thread ht_malloc_func_t real_malloc = NULL;
__thread ht_realloc_func_t real_realloc = NULL;
__thread ht_free_func_t real_free = NULL;

static void ht_init_real_funcs()
{
	if (unlikely(real_malloc == NULL)) {
		real_malloc = (ht_malloc_func_t)dlsym(RTLD_NEXT, "malloc");
		if (real_malloc == NULL) {
			abort();
		}
	}

	if (unlikely(real_realloc == NULL)) {
		real_realloc = (ht_realloc_func_t)dlsym(RTLD_NEXT, "realloc");
		if (real_malloc == NULL) {
			abort();
		}
	}

	if (unlikely(real_free == NULL)) {
		real_free = (ht_free_func_t)dlsym(RTLD_NEXT, "free");
		if (real_free == NULL) {
			abort();
		}
	}
}

ht_malloc_func_t ht_get_real_malloc()
{
	ht_init_real_funcs();

	return real_malloc;
}

ht_realloc_func_t ht_get_real_realloc()
{
	ht_init_real_funcs();

	return real_realloc;
}

ht_free_func_t ht_get_real_free()
{
	ht_init_real_funcs();

	return real_free;
}
