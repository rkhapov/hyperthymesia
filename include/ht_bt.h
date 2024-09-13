#ifndef HYPERTHYMESIA_BT_H
#define HYPERTHYMESIA_BT_H

#include <stdint.h>

typedef void *ht_bt_entry_t;

typedef struct ht_backtrace {
	size_t size;
	// HT_MAX_BT_DEPTH is defined in CMakeLists.txt
	ht_bt_entry_t entries[HT_MAX_BT_DEPTH];
} ht_backtrace_t;

int ht_bt_collect(ht_backtrace_t *bt);

uint32_t ht_bt_get_hash(const ht_backtrace_t *bt);

#endif // HYPERTHYMESIA_BT_H
