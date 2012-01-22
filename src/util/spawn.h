#ifndef spawnhincluded
#define spawnhincluded

#include <./util/config.h>

typedef void * (* treepreroutine)
(
	const unsigned, const runconfig *const, void *const
);

typedef void (* treeroutine)(void *const);

extern void treespawn(treepreroutine, treeroutine, const runconfig *const);

#endif
