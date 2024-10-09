#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stddef.h>
#include <dlfcn.h>

#include "ht_hints.h"

typedef enum {
	HINTFN_UNINITIALIZED = 0,
	HINTFN_INITIALIZED = 1,
} hintfn_status_t;

static void empty_stack_location_hint(__attribute__((unused)) void **begin,
				      __attribute__((unused)) void **end)
{
	*begin = NULL;
	*end = NULL;
}

__thread stack_hintfn_t stack_hintfn = empty_stack_location_hint;

void ht_call_stack_location_hint(void **begin, void **end)
{
	stack_hintfn(begin, end);
}

void ht_register_stack_location_hint(stack_hintfn_t fn)
{
	stack_hintfn = fn;
}
