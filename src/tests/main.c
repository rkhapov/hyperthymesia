extern void test_ht_malloc_ht_free();
extern void test_malloc_benchmark();

int main(int argc, char **argv)
{
	test_ht_malloc_ht_free();

	test_malloc_benchmark();

	return 0;
}
