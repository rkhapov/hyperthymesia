extern void test_ht_malloc_ht_free();
extern void test_malloc_benchmark();
extern void test_ht_bt_collect();

int main()
{
	test_ht_malloc_ht_free();

	test_malloc_benchmark();

	test_ht_bt_collect();

	return 0;
}
