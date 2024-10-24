#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <malloc.h>
#include <time.h>

#include <sys/mman.h>

#include "ht_alloc_header.h"
#include "ht_bt.h"
#include "ht_table.h"
#include "ht_real_funcs.h"
#include "ht_log.h"
#include "ht_malloc.h"
#include "ht_rand.h"

static void *mmap_malloc(size_t size)
{
	void *addr = mmap(NULL, size, PROT_READ | PROT_WRITE,
			  MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (addr != MAP_FAILED) {
		return addr;
	}

	return NULL;
}

static void mmap_free(void *addr, size_t size)
{
	// NULL check is performed in free

	munmap(addr, size);
}

static void *mmap_realloc(void *old_addr, size_t old_size, size_t new_size)
{
	// NULL check is performed in realloc

	void *new_addr = mmap_malloc(new_size);
	if (new_addr == NULL) {
		return NULL;
	}

	memcpy(new_addr, old_addr, old_size);

	mmap_free(old_addr, old_size);

	return new_addr;
}

static void *do_real_allocation(int managed, size_t size)
{
	if (__builtin_expect(managed, 1)) {
		ht_malloc_func_t real_malloc = ht_get_real_malloc();

		return real_malloc(size);
	}

	return mmap_malloc(size);
}

static void do_real_deallocation(int managed, void *addr, size_t size)
{
	if (__builtin_expect(managed, 1)) {
		ht_free_func_t real_free = ht_get_real_free();

		real_free(addr);
	} else {
		mmap_free(addr, size);
	}
}

static void *do_real_reallocation(int managed, void *addr, size_t old_size,
				  size_t new_size)
{
	if (__builtin_expect(managed, 1)) {
		ht_realloc_func_t real_realloc = ht_get_real_realloc();

		return real_realloc(addr, new_size);
	}

	return mmap_realloc(addr, old_size, new_size);
}

// we not interesting in registering recursive malloc
// ex: mallocs in stack unwiding functions
// so we will manage only mallocs with depth = 1
__thread int malloc_depth = 0;

#if HT_ALLOC_SAMPLES > 0 && HT_ALLOC_SAMPLES < 100
static pthread_once_t rng_init_once_control = PTHREAD_ONCE_INIT;

static void init_rng()
{
	ht_rand_seed(time(NULL));
}
#endif

static int is_allocation_in_sample()
{
#if HT_ALLOC_SAMPLES >= 100
	return 1;
#elif HT_ALLOC_SAMPLES <= 0
	return 0;
#else
	(void)pthread_once(&rng_init_once_control, init_rng);

	return (ht_rand_get() % 100) < HT_ALLOC_SAMPLES;
#endif
}

void *malloc(size_t size)
{
	malloc_depth++;

	int managed = malloc_depth == 1;

	const size_t total_size = sizeof(ht_alloc_header_t) + size;
	void *raw = do_real_allocation(managed, total_size);
	if (raw == NULL) {
		malloc_depth--;
		return NULL;
	}

	ht_alloc_header_t *header = raw;

	header->managed = managed && is_allocation_in_sample();
	header->alloc_size = size;

	if (header->managed) {
		if (ht_bt_collect(&header->alloc_bt, 2)) {
			// TODO: do smth more supportable here ?
			ht_log_stderr("can't get backtrace at malloc");
			abort();
		}

		ht_table_register_allocation(&header->alloc_bt, size);
	}

	malloc_depth--;

	return (void *)(header + 1);
}

void *calloc(size_t nmemb, size_t size)
{
	void *ptr = malloc(nmemb * size);
	if (ptr != NULL) {
		memset(ptr, 0, nmemb * size);
	}

	return ptr;
}

void free(void *ptr)
{
	if (ptr == NULL) {
		return;
	}

	ht_alloc_header_t *header = ptr;
	header--;

	if (header->managed) {
		ht_table_register_deallocation(&header->alloc_bt,
					       header->alloc_size);
	}

	do_real_deallocation(header->managed, (void *)header,
			     header->alloc_size);
}

void *realloc(void *ptr, size_t size)
{
	// realloc-compatibility
	if (ptr == NULL) {
		return malloc(size);
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

	header = do_real_reallocation(header->managed, header,
				      header->alloc_size,
				      sizeof(ht_alloc_header_t) + size);
	if (header == NULL) {
		return NULL;
	}

	if (header->managed) {
		ht_table_register_deallocation(&header->alloc_bt,
					       header->alloc_size);

		if (ht_bt_collect(&header->alloc_bt, 2)) {
			// extremely unlikely...
			// TODO: do smth more supportable here ?
			ht_log_stderr("can't get backtrace at realloc");
			abort();
		}

		ht_table_register_allocation(&header->alloc_bt, size);
	}

	header->alloc_size = size;

	return (void *)(header + 1);
}

void *reallocarray(void *ptr, size_t nmemb, size_t size)
{
	return realloc(ptr, nmemb * size);
}