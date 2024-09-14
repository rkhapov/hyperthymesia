#ifndef HYPERTHYMESIA_TABLE_H
#define HYPERTHYMESIA_TABLE_H

#include <stddef.h>
#include <pthread.h>

#include "ht_alloc_stat.h"

typedef struct ht_allocation_bucket {
	size_t used;
	size_t capacity;
	ht_alloc_stat_t *stats;
} ht_allocation_bucket_t;

typedef struct ht_allocation_table {
	size_t buckets_count;
	ht_allocation_bucket_t *buckets;
	pthread_mutex_t mutex;
} ht_allocation_table_t;

int ht_thread_table_init(size_t buckets_count, size_t bucket_start_capacity);

ht_alloc_stat_t *ht_table_get_allocation_stats(const ht_backtrace_t *bt);

int ht_thread_table_destroy();

typedef void (*ht_alloc_stat_callback_t)(const ht_alloc_stat_t *stat);

void ht_table_foreach_stat(ht_allocation_table_t *table, ht_alloc_stat_callback_t cb);

ht_allocation_table_t *ht_get_table_of_current_thread();

void ht_table_lock(ht_allocation_table_t *table);

void ht_table_unlock(ht_allocation_table_t *table);

#endif // HYPERTHYMESIA_TABLE_H
