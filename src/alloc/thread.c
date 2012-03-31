#include <./alloc/worker.h>
#include <./util/spawn.h>

#include <echotwo.h>

#include <string.h>
#include <stdio.h>
#include <sched.h>

static void routine(const void *const arg)
{
	const idargument *const ia = (idargument *)arg;

	rnode * rings[nrings];
	bzero(rings, sizeof(rings));

	worker(ia->tp->rc, rings, ia->id);

	freerings(rings);

	printf("work %03u is done on core %d\n", ia->id, sched_getcpu());
}

int main(const int argc, const char *const argv[])
{
	const treeplugin tp = {
		.makeargument = makeidargument,
		.dropargument = dropidargument,
		.treeroutine = routine,
		.extra = NULL,
		.rc = formconfig(argc, argv, 64, 1024 * 1024) };

	treespawn(&tp);

	freeconfig((runconfig *)tp.rc);

	return 0;
}
