#ifndef HYPERTHYMESIA_TABLE_H
#define HYPERTHYMESIA_TABLE_H

#include <stddef.h>
#include <pthread.h>

#include "ht_alloc_stat.h"

typedef struct ht_allocation_bucket {
	size_t used;
	size_t capacity;
	pthread_mutex_t mutex;
	ht_alloc_stat_t *stats;
} ht_allocation_bucket_t;

typedef struct ht_allocation_table {
	size_t buckets_count;
	ht_allocation_bucket_t *buckets;
} ht_allocation_table_t;

typedef void (*ht_alloc_stat_callback_t)(const ht_alloc_stat_t *stat);

int ht_table_init(size_t buckets_count, size_t bucket_start_capacity);

void ht_table_register_allocation(const ht_backtrace_t *bt, size_t size);

void ht_table_register_deallocation(const ht_backtrace_t *bt, size_t size);

int ht_table_destroy();

void ht_table_foreach_stat(ht_alloc_stat_callback_t cb);

#endif // HYPERTHYMESIA_TABLE_H
