#include <./util/config.h>
#include <./util/echotwo.h>

#include <stdlib.h>
#include <unistd.h>
#include <sched.h>

static runconfig * allocforncores(void)
{
	const unsigned ncoresmax = CPU_SETSIZE;
	eprintf("assuming no more than %u cores\n", ncoresmax);

	cpu_set_t cpuset;
	if(!sched_getaffinity(getpid(), ncoresmax, &cpuset)) { } else
	{
		fail("can't get current affinity");
	}
	
	const int ncores = CPU_COUNT(&cpuset);

	eprintf("ncores %u\n", ncores);

	runconfig *const cfg =
		malloc(sizeof(runconfig)
			+ sizeof(unsigned) * (ncores - 1));

	if(cfg) { } else
	{
		fail("can't allocate memory for config structure");
	}

	cfg->ncores = ncores;

	unsigned cc = 0; // current core
	for(unsigned i = 0; cc < ncores; i += 1)
	{
		if(CPU_ISSET(i, &cpuset))
		{
			cfg->corelist[cc] = i;
			cc += 1;
		}
	}

	return cfg;
}

runconfig * formconfig(const int argc, const char *const argv[],
	const unsigned defsz, const unsigned defnw)
{
	runconfig *const cfg = allocforncores();

	cfg->nworkers = defnw;
	cfg->size = defsz;

	eprintf("configured"
		"\n\tworkers: %u"
		"\n\tsize: %u"
		"\n\tsz/wrk: %u"
		"\n\tflags: %u"
		"\n\tpagelength: %u"
		"\n\tcorelist (%u):",
		cfg->nworkers, cfg->size, cfg->size/cfg->nworkers,
		cfg->flags, cfg->pagelength, cfg->ncores);

	for(unsigned i = 0; i < cfg->ncores; i += 1)
	{
		eprintf(" %u", cfg->corelist[i]);
	}

	eprintf("\n");

	return cfg;
}

void freeconfig(runconfig *const rc)
{
	if(rc)
	{
		free(rc);
	}
}
