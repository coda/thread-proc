#ifndef muljobhincluded
#define muljobhincluded

#include <util/config.h>

typedef struct
{
	unsigned mapoffset;
	unsigned maplength;
	unsigned baseoffset;
	unsigned baserow;
	unsigned nrows;
	unsigned absolutebaserow;
} joblayout;

extern joblayout definejob(
	const runconfig *const cfg,
	const unsigned id,
	const unsigned l, const unsigned m,
	const unsigned tr, const unsigned tc);

#endif
