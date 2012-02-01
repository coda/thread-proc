#ifndef spawnhincluded
#define spawnhincluded

#include <./util/config.h>

typedef struct treeplugintag
{
	void * (* makeargument)(
		const struct treeplugintag *const tp,
		const unsigned id,
		const void *const parentarg);

	void (* dropargument)(void *const arg);
	void (* treeroutine)(const void *const arg);

	const runconfig *const rc;
	void * extra;
} treeplugin;

extern void treespawn(const treeplugin *const);

typedef struct
{
	unsigned id;
	const treeplugin * tp;
} idargument;

extern void * makeidargument(
	const treeplugin *const tp,
	const unsigned id,
	const void *const parentarg);

extern void dropidargument(void *const arg);

#endif
