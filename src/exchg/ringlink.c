#include <exchg/ringlink.h>

void rlform(ringlink *const rl, const int toread, const int towrite)
{
	rl->toread = toread;
	rl->towrite = towrite;
	rl->writable = 1;
	rl->nexchanges = 0;
}

void rldrop(ringlink *const rl)
{
}

void rlwrite(ringlink *const rl, const unsigned i)
{
	
}
