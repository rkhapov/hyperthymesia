#ifndef HYPERTHYMESIA_ALLOC_STAT_H
#define HYPERTHYMESIA_ALLOC_STAT_H

#include "ht_bt.h"

typedef unsigned long long stat_counter_t;

typedef struct ht_alloc_stat {
	stat_counter_t alloc_count;
	stat_counter_t free_count;
	stat_counter_t size;
	ht_backtrace_t bt;
} ht_alloc_stat_t;

#endif // HYPERTHYMESIA_ALLOC_STAT_H
