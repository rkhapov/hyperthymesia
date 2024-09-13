#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ht_bt.h"

__attribute__((noinline)) void baz()
{
	char some_vars[1024];
	memset(some_vars, 'c', sizeof(some_vars));

	ht_backtrace_t bt;
	if (ht_bt_collect(&bt) != 0) {
		perror("ht_bt_collect");
		abort();
	}

	printf("use addr2line to check if it is valid bt:\n");
	for (size_t i = 0; i < bt.size; ++i) {
		printf("%p\n", bt.entries[i]);
	}
	printf("end of bt\n");
}

__attribute__((noinline)) void bar()
{
	char some_vars[1024];
	memset(some_vars, 'b', sizeof(some_vars));

	baz();
}

__attribute__((noinline)) void foo()
{
	char some_vars[1024];
	memset(some_vars, 'a', sizeof(some_vars));

	bar();
}

void test_ht_bt_collect()
{
	foo();
}