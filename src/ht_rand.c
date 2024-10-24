#include "ht_rand.h"

__thread unsigned long next = 1;

int ht_rand_get()
{
	next = next * 1103515245 + 12345;
	return (unsigned int)(next / 65536) % (HT_RAND_MAX + 1);
}

void ht_rand_seed(unsigned long seed)
{
	next = seed;
}
