#include <stdio.h>

extern void test_ht_malloc_ht_free();
extern void test_malloc_benchmark();
extern void test_realloc_benchmark();
extern void test_ht_bt_collect();
extern void test_ht_realloc();

void do_run_test(void (*fn)(), const char *name)
{
	printf("Running test %s...\n", name);
	fn();
	printf("Test %s done\n", name);
	printf("---------------------\n");
}

#define run_test(name) do_run_test(name, #name)

int main()
{
	run_test(test_ht_bt_collect);

	run_test(test_ht_malloc_ht_free);

	run_test(test_ht_realloc);

	run_test(test_malloc_benchmark);

	run_test(test_realloc_benchmark);

	return 0;
}
