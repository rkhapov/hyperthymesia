#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#include "ht_table.h"

ht_allocation_table_t global_allocations_table;

int ht_table_init(size_t buckets_count, size_t bucket_start_capacity)
{
	if (global_allocations_table.buckets_count != 0) {
		return -1;
	}

	global_allocations_table.buckets = (ht_allocation_bucket_t *)malloc(
		sizeof(ht_allocation_bucket_t) * buckets_count);

	if (global_allocations_table.buckets == NULL) {
		return -1;
	}

	for (size_t i = 0; i < buckets_count; ++i) {
		ht_allocation_bucket_t *bucket =
			&global_allocations_table.buckets[i];

		pthread_mutex_init(&global_allocations_table.buckets[i].mutex,
				   NULL);

		bucket->used = 0;
		bucket->capacity = bucket_start_capacity;
		bucket->stats = (ht_alloc_stat_t *)malloc(
			sizeof(ht_alloc_stat_t) * bucket_start_capacity);

		if (bucket->stats == NULL) {
			ht_table_destroy();
			return -1;
		}

		++global_allocations_table.buckets_count;
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

static ht_allocation_bucket_t *get_bucket(const ht_backtrace_t *bt)
{
	const uint32_t bt_hash = ht_bt_get_hash(bt);
	const size_t bucket_idx =
		(size_t)bt_hash % global_allocations_table.buckets_count;
	ht_allocation_bucket_t *bucket =
		&global_allocations_table.buckets[bucket_idx];

	return bucket;
}

void ht_table_register_allocation(const ht_backtrace_t *bt, size_t size)
{
	ht_allocation_bucket_t *bucket = get_bucket(bt);

	pthread_mutex_lock(&bucket->mutex);

	ht_alloc_stat_t *stat = find_or_append_stats(bucket, bt);
	if (stat == NULL) {
		// TODO: do some more supportable here ???
		perror("can't find or create stat for bt");
		abort();
	}

	stat->alloc_count++;
	stat->total_size += size;

	pthread_mutex_unlock(&bucket->mutex);
}

void ht_table_register_deallocation(const ht_backtrace_t *bt, size_t size)
{
	ht_allocation_bucket_t *bucket = get_bucket(bt);

	pthread_mutex_lock(&bucket->mutex);

	ht_alloc_stat_t *stat = find_or_append_stats(bucket, bt);
	if (stat == NULL) {
		// TODO: do some more supportable here ???
		perror("can't find or create stat for bt");
		abort();
	}

	stat->free_count++;
	stat->total_size -= size;

	pthread_mutex_unlock(&bucket->mutex);
}

int ht_table_destroy()
{
	for (size_t i = 0; i < global_allocations_table.buckets_count; ++i) {
		pthread_mutex_destroy(
			&global_allocations_table.buckets[i].mutex);

		free(global_allocations_table.buckets[i].stats);
	}

	free(global_allocations_table.buckets);

	global_allocations_table.buckets = NULL;
	global_allocations_table.buckets_count = 0;

	return 0;
}

void ht_table_foreach_stat(ht_alloc_stat_callback_t cb)
{
	const size_t bc = global_allocations_table.buckets_count;
	ht_allocation_bucket_t *buckets = global_allocations_table.buckets;

	for (size_t i = 0; i < bc; ++i) {
		ht_allocation_bucket_t *bucket = &buckets[i];

		pthread_mutex_lock(&bucket->mutex);

		const ht_alloc_stat_t *stats = bucket->stats;
		const size_t used = bucket->used;

		for (size_t j = 0; j < used; ++j) {
			cb(&stats[j]);
		}

		pthread_mutex_unlock(&bucket->mutex);
	}
}
