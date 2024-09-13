#include <stdlib.h>

#include "ht_table.h"

__thread ht_allocation_table_t thread_table;

int ht_thread_table_init(size_t buckets_count, size_t bucket_start_capacity)
{
	if (thread_table.buckets_count != 0) {
		return -1;
	}

	thread_table.buckets = (ht_allocation_bucket_t *)malloc(
		sizeof(ht_allocation_bucket_t) * buckets_count);

	if (thread_table.buckets == NULL) {
		return -1;
	}

	for (size_t i = 0; i < buckets_count; ++i) {
		ht_allocation_bucket_t *bucket = &thread_table.buckets[i];

		bucket->used = 0;
		bucket->capacity = bucket_start_capacity;
		bucket->stats = (ht_alloc_stat_t *)malloc(
			sizeof(ht_alloc_stat_t) * bucket_start_capacity);

		if (bucket->stats == NULL) {
			ht_thread_table_destroy();
			return -1;
		}

		++thread_table.buckets_count;
	}

	return 0;
}

int ht_thread_table_destroy()
{
	for (size_t i = 0; i < thread_table.buckets_count; ++i) {
		free(thread_table.buckets[i].stats);
	}

	free(thread_table.buckets);
	return 0;
}
