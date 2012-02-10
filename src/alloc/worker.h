#ifndef workerhincluded
#define workerhincluded

#include <./util/config.h>

typedef struct rnodetag
{
	struct rnodetag* next;
	struct rnodetag* prev;
} rnode;

enum { nrings = 256 };

extern void worker(const runconfig *const rc, rnode * rings[],
	const unsigned id);

extern void freerings(rnode * rings[]);

#endif
