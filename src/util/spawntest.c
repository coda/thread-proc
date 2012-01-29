#include <./util/spawn.h>
#include <./util/config.h>
#include <./util/echotwo.h>

#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>
#include <sched.h>

typedef struct
{
	unsigned id;
} arguments;

static void * makeargument(const unsigned id, const runconfig *const rc,
	const void *const prevarg)
{
	arguments *const arg = malloc(sizeof(arguments));

	if(arg) { } else
	{
		fail("preroutine can't allocate next arg");
	}

	arg->id = id;
	
	return arg;
}

static void freeargument(void *const argument)
{
	free(argument);
}

static void routine(const void *const argptr)
{
	const arguments *const arg = argptr;
	sleep((arg->id + 1) / 4);
	printf("routine %04u is done on core %d\n", arg->id, sched_getcpu());
}


int main(const int argc, const char *const argv[])
{
	const runconfig *const rc = formconfig(argc, argv, 64, 2048);
	const treeplugin tp =
		{ .makeargument = makeargument,
			.dropargument = freeargument, .treeroutine = routine };

	treespawn(&tp, rc);

	freeconfig((runconfig *)rc);
	return 0;
}