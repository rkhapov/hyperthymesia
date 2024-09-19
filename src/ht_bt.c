#include <stdlib.h>
#include <string.h>
#include <execinfo.h>

#include "ht_hints.h"
#include "ht_bt.h"

static void fill_bt_backtrace(ht_backtrace_t *bt, int skip)
{
	void *buff[HT_MAX_BT_DEPTH + 2];
	int read = backtrace((void **)&buff, HT_MAX_BT_DEPTH + 2);
	bt->size = ((size_t)read - skip) > HT_MAX_BT_DEPTH ?
			   HT_MAX_BT_DEPTH :
			   ((size_t)read - skip);
	memcpy(bt->entries, buff + skip, bt->size * sizeof(void *));
}

static size_t fill_bt_unwide(ht_backtrace_t *bt, int skip)
{
	/*
	 * How it works
	 * 
	 * Consider the stack of the current function f_0
	 * (where f_1 is caller of f_0, f_2 is caller of f_1 and so on)
	 * rbp is the pointer of the function frame
	 * 
	 * The stack looks like:
	 * low addr
	 *      stack_begin
	 * 
	 *      [vars of f_0 ]
	 *    +-[rbp of f_1  ]       <- frame of f_0 (~ rbp)
	 *    | [ret from f_0]
	 *    | [vars of f_1 ]
	 *  +-+>[rbp of f_2  ]       <- frame of f_1 (~ *rbp)
	 *  |   [ret from f_1]
	 *  |   [vars of f_2 ]
	 *  +-->[rbp of f_3  ]       <- frame of f_2 (~ *(*rbp))
	 *      [ret from f_2]
	 * 
	 *      stack_end
	 * high addr
	 * 
	 * So, to unwide the stack, we must firstly obtain address of
	 * the frame of current function (~ read rbp register)
	 * 
	 * Then, sub from the obtained value 8 bytes, and get the ret addr
	 * Move the frame pointer to the value at the top of current frame pointer,
	 * which gives us the rbp from previous function
	 * 
	 * Cycle it until needed depth or bounds of the stack are reached
	 * This will only works when functions are not chain optimized and frame
	 * pointers are not ommited
	 * 
	 * The implementation is inspired by gpdb stack unwiding
	*/

	uintptr_t stack_begin, stack_end;
	ht_call_stack_location_hint((void **)&stack_begin, (void **)&stack_end);
	if (stack_begin == 0 || stack_end == 0) {
		return 0UL;
	}

	uintptr_t current_function_rbp = (uintptr_t)__builtin_frame_address(0);
	uintptr_t *next_function_rbp_ptr = (uintptr_t *)current_function_rbp;

	for (int i = 0; i < HT_MAX_BT_DEPTH + skip; ++i) {
		uintptr_t next_function_rbp = *next_function_rbp_ptr;
		// bounds of the stack are reached
		if (next_function_rbp > stack_end ||
		    next_function_rbp < stack_begin) {
			break;
		}
		// somehow we go back if the backtrace
		if (next_function_rbp < (uintptr_t)next_function_rbp_ptr) {
			break;
		}

		if (skip > 0) {
			--skip;
		} else {
			uintptr_t *return_addr_ptr =
				(uintptr_t *)(next_function_rbp_ptr + 1);
			bt->entries[bt->size++] = (void *)*return_addr_ptr;
		}

		next_function_rbp_ptr = (uintptr_t *)next_function_rbp;
	}

	return bt->size;
}

int ht_bt_collect(ht_backtrace_t *bt, int skip)
{
	memset(bt, 0, sizeof(ht_backtrace_t));

	// firstly use unwide because it is much faster than backtrace fn

	if (__builtin_expect(fill_bt_unwide(bt, skip) > 0, 1)) {
		return 0;
	}

	fill_bt_backtrace(bt, skip);

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
