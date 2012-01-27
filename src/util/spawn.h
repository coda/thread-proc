#ifndef spawnhincluded
#define spawnhincluded

#include <./util/config.h>

typedef void * (* treepreroutine)(const unsigned id, const runconfig *const rc,
	void *const parentarg);

typedef void (* treeroutine)(void *const arg);

extern void treespawn(const treepreroutine, const treeroutine,
	const runconfig *const);

#endif
