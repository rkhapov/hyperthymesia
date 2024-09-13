#include <string.h>
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

static ht_alloc_stat_t *find_stats_in_bucket(ht_allocation_bucket_t *bucket,
					     const ht_backtrace_t *bt)
{
	const size_t used = bucket->used;

	for (size_t i = 0; i < used; ++i) {
		ht_alloc_stat_t *stat = &bucket->stats[i];
		if (ht_bt_equals(&stat->bt, bt)) {
			return stat;
		}
	}

	return NULL;
}

static ht_alloc_stat_t *append_stat_for_new_bt(ht_allocation_bucket_t *bucket,
					       const ht_backtrace_t *bt)
{
	if (bucket->used >= bucket->capacity) {
		bucket->capacity *= 2;
		bucket->stats = (ht_alloc_stat_t *)realloc(
			bucket->stats,
			bucket->capacity * sizeof(ht_alloc_stat_t));
		if (bucket->stats == NULL) {
			return NULL;
		}
	}

	ht_alloc_stat_t *stats = &bucket->stats[bucket->used++];

	memset(stats, 0, sizeof(ht_alloc_stat_t));
	memcpy(&stats->bt, bt, sizeof(ht_backtrace_t));

	return stats;
}

static ht_alloc_stat_t *find_or_append_stats(ht_allocation_bucket_t *bucket,
					     const ht_backtrace_t *bt)
{
	ht_alloc_stat_t *stat = find_stats_in_bucket(bucket, bt);
	if (stat != NULL) {
		return stat;
	}

	return append_stat_for_new_bt(bucket, bt);
}

ht_alloc_stat_t *ht_table_get_allocation_stats(const ht_backtrace_t *bt)
{
	if (thread_table.buckets_count == 0) {
		return NULL;
	}

	const uint32_t bt_hash = ht_bt_get_hash(bt);
	const size_t bucket_idx = (size_t)bt_hash % thread_table.buckets_count;
	ht_allocation_bucket_t *bucket = &thread_table.buckets[bucket_idx];

	return find_or_append_stats(bucket, bt);
}

int ht_thread_table_destroy()
{
	for (size_t i = 0; i < thread_table.buckets_count; ++i) {
		free(thread_table.buckets[i].stats);
	}

	free(thread_table.buckets);
	return 0;
}
