#include <./util/tools.h>

#include <echotwo.h>

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

unsigned groupofid(
	const unsigned nitems, const unsigned ngroups, const unsigned id)
{
	if(nitems && ngroups && id < nitems) { } else
	{
		fail("group calculation is a bug for "
			"nitems = %u; ngroups = %u; id = %u",
			nitems, ngroups, id);
	}

	const unsigned fix = nitems % ngroups != 0;

	// full group size
	const unsigned fgsz = nitems / ngroups + fix;

	// full groups count
	const unsigned nfg = nitems % ngroups;

	// common full groups length
	const unsigned cfgl = nfg * fgsz;

	unsigned grp;

	if(id < cfgl)
	{
		grp = id / fgsz;
	}
	else
	{
		grp = nfg + (id - cfgl) / (fgsz - fix);
	}

	return grp;
}
