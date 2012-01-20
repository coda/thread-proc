#include <./util/spawn.h>

extern void treespawn
(
	void * (*const preroutine)
	(
		const unsigned, const runconfig *const, void *const
	),
	void (*const routine)(void *const),
	const unsigned id,
	void *const arg,
	const runconfig *const rc
) {
	

	pid_t procs[2];
	const unsigned nprocs = sizeof(procs) / sizeof(pid_t);

	void * newarg = preroutine(id, rc, arg);
}
