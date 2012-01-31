#include <./alloc/worker.h>
#include <./util/spawn.h>
#include <./util/echotwo.h>

#include <string.h>
#include <stdio.h>

#include <sched.h>

static void routine(const void *const arg)
{
	const idargument *const ia = (idargument *)arg;

//	eprintf("work %u is here\n", ia->id);

	rnode * rings[nrings];
	bzero(rings, sizeof(rings));

	worker(ia->rc, rings, ia->id);

	freerings(rings);

	printf("work %u is done on core %d\n", ia->id, sched_getcpu());
}

int main(const int argc, const char *const argv[])
{
	const treeplugin tp = {
		.makeargument = makeidargument,
		.dropargument = dropidargument,
		.treeroutine = routine,
		rc = formconfig(argc, argv, 64, 64 * 1024) };

	treespawn(&tp);
	freeconfig(tp.rc);

	return 0;
}
