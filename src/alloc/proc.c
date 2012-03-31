#include <./alloc/worker.h>
#include <./util/spawn.h>

#include <echotwo.h>

#include <string.h>
#include <stdio.h>

#include <sched.h>

static rnode * rings[nrings];

static void routine(const void *const arg)
{
	const idargument *const ia = (idargument *)arg;
	worker(ia->tp->rc, rings, ia->id);
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
