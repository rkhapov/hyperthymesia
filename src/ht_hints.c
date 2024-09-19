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

// static stack_hintfn_t get_stack_hintfn() {
//     if (__builtin_expect(stack_hintfn_status == HINTFN_UNINITIALIZED, 0)) {
//         stack_hintfn = (stack_hintfn_t) dlsym(RTLD_DEFAULT, "ht_stack_location_hint");
//         if (stack_hintfn == NULL) {
//             __attribute__((unused)) char *msg = dlerror();
//             stack_hintfn = empty_stack_location_hint;
//         }
//         stack_hintfn_status = HINTFN_INITIALIZED;

//         return stack_hintfn;
//     }

//     return stack_hintfn;
// }

void ht_call_stack_location_hint(void **begin, void **end)
{
	stack_hintfn(begin, end);
}

void ht_register_stack_location_hint(stack_hintfn_t fn)
{
	stack_hintfn = fn;
}
