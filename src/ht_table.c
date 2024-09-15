#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#include "ht_table.h"
#include "ht_log.h"
#include "ht_real_funcs.h"

static ht_allocation_table_t global_allocations_table;
static pthread_once_t table_init_once_control = PTHREAD_ONCE_INIT;

const char *buckets_count_var_name = "HT_BUCKETS";
const char *bucket_init_len_var_name = "HT_BUCKET_LEN";

#define ensure_table_is_initialized() \
	((void)pthread_once(&table_init_once_control, table_init))

static size_t read_variable(const char *name, size_t default_value)
{
	char *sval = getenv(name);
	if (sval == NULL) {
		return default_value;
	}

	long long v = atoll(sval);
	if (v > 0) {
		return (size_t)v;
	}

	return default_value;
}

static void log_var_value(const char *name, size_t value)
{
	char buf[128];
	sprintf(buf, "%s = %zu", name, value);

	ht_log_stderr(buf);
}

static void table_init()
{
	size_t buckets_count = read_variable(buckets_count_var_name, 47351);
	log_var_value(buckets_count_var_name, buckets_count);

	size_t bucket_start_capacity =
		read_variable(bucket_init_len_var_name, 32);
	log_var_value(bucket_init_len_var_name, bucket_start_capacity);

	ht_malloc_func_t real_malloc = ht_get_real_malloc();

	global_allocations_table.buckets =
		(ht_allocation_bucket_t *)real_malloc(
			sizeof(ht_allocation_bucket_t) * buckets_count);

	if (global_allocations_table.buckets == NULL) {
		return;
	}

	for (size_t i = 0; i < buckets_count; ++i) {
		ht_allocation_bucket_t *bucket =
			&global_allocations_table.buckets[i];

		pthread_mutex_init(&global_allocations_table.buckets[i].mutex,
				   NULL);

		bucket->used = 0;
		bucket->capacity = bucket_start_capacity;
		bucket->stats = (ht_alloc_stat_t *)real_malloc(
			sizeof(ht_alloc_stat_t) * bucket_start_capacity);

		if (bucket->stats == NULL) {
			abort();
		}

		++global_allocations_table.buckets_count;
	}
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
	ht_realloc_func_t real_realloc = ht_get_real_realloc();

	if (bucket->used >= bucket->capacity) {
		bucket->capacity *= 2;
		bucket->stats = (ht_alloc_stat_t *)real_realloc(
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
	ensure_table_is_initialized();

	ht_allocation_bucket_t *bucket = get_bucket(bt);

	pthread_mutex_lock(&bucket->mutex);

	ht_alloc_stat_t *stat = find_or_append_stats(bucket, bt);
	if (stat == NULL) {
		// TODO: do some more supportable here ???
		ht_log_stderr("can't find or create stat for bt");
		abort();
	}

	stat->alloc_count++;
	stat->total_size += size;

	pthread_mutex_unlock(&bucket->mutex);
}

void ht_table_register_deallocation(const ht_backtrace_t *bt, size_t size)
{
	ensure_table_is_initialized();

	ht_allocation_bucket_t *bucket = get_bucket(bt);

	pthread_mutex_lock(&bucket->mutex);

	ht_alloc_stat_t *stat = find_or_append_stats(bucket, bt);
	if (stat == NULL) {
		// TODO: do some more supportable here ???
		ht_log_stderr("can't find or create stat for bt");
		abort();
	}

	stat->free_count++;
	stat->total_size -= size;

	pthread_mutex_unlock(&bucket->mutex);
}

void ht_table_foreach_stat(ht_alloc_stat_callback_t cb)
{
	ensure_table_is_initialized();

	const size_t buf_size = 64;
	ht_alloc_stat_t buf[buf_size];

	const size_t bc = global_allocations_table.buckets_count;
	ht_allocation_bucket_t *buckets = global_allocations_table.buckets;

	for (size_t i = 0; i < bc; ++i) {
		ht_allocation_bucket_t *bucket = &buckets[i];

		// here we must read to buf by some portions
		// because we dont know how long cb execution is

		// bucket->used only increases
		// so there is no point in reading it with capturing the mutex
		// the worst thing that can happen - not reading some
		// last allocations
		size_t used = bucket->used;

		size_t begin = 0;
		while (begin < used) {
			size_t end = begin + buf_size;
			if (end > used) {
				end = used;
			}

			pthread_mutex_lock(&bucket->mutex);

			ht_alloc_stat_t *stats = bucket->stats;

			size_t part_size = end - begin;
			memcpy(buf, stats + begin,
			       part_size * sizeof(ht_alloc_stat_t));

			begin = end;

			pthread_mutex_unlock(&bucket->mutex);

			for (size_t i = 0; i < part_size; ++i) {
				cb(&buf[i]);
			}
		}
	}
}
