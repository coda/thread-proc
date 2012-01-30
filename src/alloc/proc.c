#include <./alloc/worker.h>
#include <./util/spawn.h>
#include <./util/echotwo.h>

#include <string.h>
#include <stdio.h>

static rnode * forkrings[NRINGS];

static void routine(const void *const arg)
{
	const idargument *const ia = (idargument *)arg;
	worker(ia->rc, forkrings, ia->id);
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
