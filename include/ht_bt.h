#ifndef HYPERTHYMESIA_BT_H
#define HYPERTHYMESIA_BT_H

#include <stdint.h>

typedef struct ht_bt_collect_config {
} ht_bt_collect_config_t;

typedef struct ht_backtrace {
	// HT_MAX_BT_DEPTH is defined in CMakeLists.txt
	uintptr_t bt[HT_MAX_BT_DEPTH];
} ht_backtrace_t;

int ht_bt_collect(const ht_bt_collect_config_t *cfg, ht_backtrace_t *bt);

#endif // HYPERTHYMESIA_BT_H
