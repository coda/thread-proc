#include <./util/tools.h>

unsigned align(const unsigned n, const unsigned blklen)
{
	const unsigned r = n % blklen;
	const unsigned t = (blklen - r) & ((unsigned)-1 + (r == 0));
	return n + t;
}

unsigned aligndown(const unsigned n, const unsigned blklen)
{
	return n - n % blklen;
}
