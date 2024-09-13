#ifndef HYPERTHYMESIA_DEFER_H
#define HYPERTHYMESIA_DEFER_H

#if defined(__GNUC__) || defined(__clang__)

#include <stdio.h>

typedef void (*defer_fn)(void *);

typedef struct defer_info {
	defer_fn function;
	void *argument;
} defer_info_t;

static inline void _ht_defer_caller(defer_info_t *info)
{
	if (info) {
		info->function(info->argument);
	}
}

#define __HT_DEFER_CONCAT(a, b) a##b
#define _HT_DEFER_CONCAT(a, b) __HT_DEFER_CONCAT(a, b)
#define ht_defer(fn, arg)                                                \
	__attribute__((cleanup(_ht_defer_caller))) defer_info_t          \
	_HT_DEFER_CONCAT(_defer_on_line, __LINE__) = { .function = (fn), \
						       .argument = (arg) }

static inline void ht_close_file(void *arg)
{
	FILE *fp = *(FILE **)arg;

	if (fp == NULL) {
		return;
	}

	fclose(fp);
}

#else

#error defer is not implemented for this compiler

#endif /* __GNUC__ or __clang__ */

#endif /* HYPERTHYMESIA_DEFER_H */