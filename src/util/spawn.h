#ifndef spawnhincluded
#define spawnhincluded

#include <./util/config.h>

extern void treespawn
(
	void * (*const preroutine)
	(
		const unsigned id,
		const runconfig *const rc
		void *const arg
	),
	void (*const routine)(void *const arg),
	const unsigned id,
	void *const arg,
	const runconfig *const rc
);

#endif
