#include <stdlib.h>
#include <string.h>
#include <execinfo.h>
#include <pthread.h>

#define UNW_LOCAL_ONLY
#include <libunwind.h>

#include "ht_bt.h"

__thread int uw_context_initialized = 0;
__thread unw_context_t context;
__thread unw_cursor_t cursor;

int ht_bt_collect(ht_backtrace_t *bt, int skip)
{
	memset(bt, 0, sizeof(ht_backtrace_t));

	if (!uw_context_initialized) {
		unw_getcontext(&context);
		unw_init_local(&cursor, &context);
		uw_context_initialized = 1;
	}

	while (unw_step(&cursor) > 0 && bt->size < HT_MAX_BT_DEPTH) {
		unw_word_t pc;
		unw_get_reg(&cursor, UNW_REG_IP, &pc);
		if (pc == 0) {
			break;
		}

		if (skip) {
			--skip;
		} else {
			bt->entries[bt->size++] = (void *)pc;
		}
	}

	// inspired by gpdb backptrace collection

	// uintptr_t current_frame = (uintptr_t)__builtin_frame_address(0);
	// void **next_frame = (void **)current_frame;

	// for (int i = 0; i < HT_MAX_BT_DEPTH; ++i) {
	// 	if (/*(uintptr_t)*next_frame > (uintptr_t)stack_bottom ||*/ // really dangerous to not check if we gone out the stack
	// 	    (uintptr_t)*next_frame < (uintptr_t)next_frame) {
	// 		break;
	// 	}

	// 	if (skip > 0) {
	// 		skip--;
	// 	} else {
	// 		uintptr_t *frame_address =
	// 			(uintptr_t *)(next_frame + 1);
	// 		bt->entries[bt->size++] = (void *)*frame_address;
	// 	}

	// 	next_frame = (void **)*next_frame;
	// }

	return 0;
}

/*
 * MurMur from https://github.com/aappleby/smhasher/blob/master/src/MurmurHash1.cpp
 * Copyright (C) Austin Appleby
 */

static const uint32_t mur_mur_seed = 0x8e51ecde;

static uint32_t get_mur_mur_hash(const void *key, int len, uint32_t seed)
{
	const unsigned int m = 0xc6a4a793;
	const int r = 16;
	unsigned int h = seed ^ (len * m);
	const unsigned char *data = (const unsigned char *)key;

	while (len >= 4) {
		unsigned int k = *(unsigned int *)data;

		h += k;
		h *= m;
		h ^= h >> 16;

		data += 4;
		len -= 4;
	}

	switch (len) {
	case 3:
		h += data[2] << 16;
		/* fall through */
	case 2:
		h += data[1] << 8;
		/* fall through */
	case 1:
		h += data[0];
		h *= m;
		h ^= h >> r;
		/* fall through */
	};

	h *= m;
	h ^= h >> 10;
	h *= m;
	h ^= h >> 17;

	return h;
}

uint32_t ht_bt_get_hash(const ht_backtrace_t *bt)
{
	return get_mur_mur_hash(bt->entries, bt->size * sizeof(ht_bt_entry_t),
				mur_mur_seed);
}

int ht_bt_equals(const ht_backtrace_t *a, const ht_backtrace_t *b)
{
	return a->size == b->size &&
	       memcmp(a->entries, b->entries,
		      a->size * sizeof(ht_bt_entry_t)) == 0;
}
