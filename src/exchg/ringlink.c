#include <exchg/ringlink.h>
#include <util/echotwo.h>
#include <unistd.h>
#include <errno.h>

void rlform(ringlink *const rl, const int readend, const int writeend)
{
	rl->readend = readend;
	rl->writeend = writeend;
	rl->writable = 1;
	rl->nexchanges = 0;
}

void rldrop(ringlink *const rl)
{
}

void rlwrite(ringlink *const rl, const unsigned i)
{
	if(rl->writable)
	{
		const int rv = write(rl->writeend, &i, sizeof(i));
		if(rv == sizeof(i))
		{
		}
		else if(errno == EPIPE)
		{
			rl->writable = 0;
		}
		else
		{
			fail("can't write to %d", rl->writeend);
		}
	}
}

unsigned rlread(ringlink *const rl)
{
	unsigned i;
	const int rv = read(rl->readend, &i, sizeof(i));
	if(rv == sizeof(i)) { } else
	{
		fail("can't read from %d", rl->readend);
	}

	return i;
}
