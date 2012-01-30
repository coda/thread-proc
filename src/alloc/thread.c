#include <./alloc/worker.h>
#include <./util/spawn.h>
#include <./util/echotwo.h>

#include <string.h>
#include <stdio.h>

#ifdef PROC
static rnode * rings[rnode];
#endif

static void routine(const void *const arg)
{
	const idargument *const ia = (idargument *)arg;

#ifndef PROC
	rnode * rings[nrings];
	bzero(rings, sizeof(rings));
#endif

	worker(ia->rc, rings, ia->id);

#ifndef PROC
	freerings(rings);
#endif

	printf("work %u is done\n", ia->id);
}

int main(const int argc, const char *const argv[])
{
	runconfig *const rc = formconfig(argc, argv, 64, 64 * 1024);

	const treeplugin tp = {
		.makeargument = makeidargument,
		.dropargument = dropidargument,
		.treeroutine = routine };

	treespawn(&tp, rc);

	freeconfig(rc);

	return 0;
}
