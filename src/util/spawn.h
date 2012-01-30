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

typedef struct
{
	unsigned id;
	const runconfig * rc;
} idargument;

extern void * makeidargument(const unsigned id, const runconfig *const rc,
	const void *const parentarg);

extern void dropidargument(void *const arg);

#endif
