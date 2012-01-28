#ifndef spawnhincluded
#define spawnhincluded

#include <./util/config.h>

typedef struct
{
	void * (* makeargument)(const unsigned id, const runconfig *const rc,
		const void *const parentarg);
	void (* dropargument)(void *const arg);
	void (* treeroutine)(const void *const arg);
} treeplugin;

extern void treespawn(const treeplugin *const, const runconfig *const);

#endif
