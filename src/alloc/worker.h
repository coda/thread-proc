#ifndef workerhincluded
#define workerhincluded

#include <./util/config.h>

typedef struct rnodetag
{
	struct rnodetag* next;
	struct rnodetag* prev;
} rnode;

#define NRINGS 256

extern const unsigned nrings;

extern void worker(const runconfig *const rc, rnode * rings[],
	const unsigned id);

extern void freerings(rnode * rings[]);

#endif
