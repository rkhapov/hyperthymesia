#include <stdlib.h>
#include <pthread.h>

#include "ht_bt.h"

__thread void *stack_bottom = NULL;

static void *get_current_thread_stack_start()
{
	pthread_attr_t attr;
	void *stackaddr;
	size_t stacksize;

	if (pthread_getattr_np(pthread_self(), &attr)) {
		return NULL;
	}

	if (pthread_attr_getstack(&attr, &stackaddr, &stacksize)) {
		return NULL;
	}

	return stackaddr + stacksize;
}

int ht_bt_collect(ht_backtrace_t *bt)
{
	if (stack_bottom == NULL) {
		stack_bottom = get_current_thread_stack_start();
		if (stack_bottom == NULL) {
			return -1;
		}
	}

	memset(bt->entries, 0, sizeof(bt->entries));

	// inspired by gpdb backptrace collection
	// we do not use backtrace() because it can be slow

	uintptr_t current_frame = (uintptr_t)__builtin_frame_address(0);
	void **next_frame = (void **)current_frame;

	for (bt->size = 0; bt->size < HT_BT_MAX_DEPTH; ++bt->size) {
		if ((uintptr_t)*next_frame > (uintptr_t)stack_bottom ||
		    (uintptr_t)*next_frame < (uintptr_t)next_frame) {
			break;
		}

		uintptr_t *frame_address = (uintptr_t *)(next_frame + 1);
		bt->entries[bt->size] = (void *)*frame_address;

		next_frame = (void **)*next_frame;
	}

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
	case 2:
		h += data[1] << 8;
	case 1:
		h += data[0];
		h *= m;
		h ^= h >> r;
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
