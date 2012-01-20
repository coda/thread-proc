#include <./util/config.h>
#include <./util/echotwo.h>

#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>
#include <sched.h>

static runconfig * allocforncores(void)
{
	const unsigned ncoresmax = 128;
	const unsigned cslen = CPU_ALLOC_SIZE(ncoresmax);
	eprintf("assuming no more than %u cores. set length = %u\n",
		ncoresmax, cslen);

	cpu_set_t * coreset = CPU_ALLOC(ncoresmax);

	if(coreset && !sched_getaffinity(getpid(), cslen, coreset)) { } else
	{
		fail("can't get current affinity");
	}
	
	const int ncores = CPU_COUNT_S(cslen, coreset);

	if(ncores) { } else
	{
		fail("don't know how to work on 0 cores\n");
	}

// 	eprintf("ncores %u\n", ncores);
// 	eprintf("allocation. sizeof(runconfig) = %u; overall = %u\n",
// 		sizeof(runconfig),
// 		sizeof(runconfig) + sizeof(unsigned) * (ncores - 1));
 
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
		if(CPU_ISSET_S(i, cslen, coreset))
		{
// 			eprintf("core %u found. current %u at offset %u\n",
// 				i, cc,
// 				(char *)&cfg->corelist[cc] - (char *)cfg);

			cfg->corelist[cc] = i;
			cc += 1;
		}
	}

	free(coreset);

//	eprintf("DONE. cfg = %p\n", cfg);

	return cfg;
}

static unsigned ispowerof2(unsigned x)
{
	return (x ^ (x - 1)) + 1 == x << 1;
}

static void configpagelen(runconfig *const cfg,
	const int argc, const char *const argv[])
{
	const long sysplen = sysconf(_SC_PAGESIZE);
	if(sysplen != -1) { } else
	{
		fail("can't get system pagesize");
	}

	unsigned flags = 0;
	unsigned plen = sysplen;

//	eprintf("argc: %u. argv: %s\n", argc, argv[3]);

	if(argc > 3)
	{
		const int i = atoi(argv[3]);
		if(i > 0 && ispowerof2(i) && i > sysplen)
		{
			plen = i;
			flags |= cfghugetlb;
		}
		else
		{
			plen = sysplen;
		}
	}

	cfg->flags = flags;
	cfg->pagelength = plen;
}

runconfig * formconfig(const int argc, const char *const argv[],
	const unsigned defnw, const unsigned defsz)
{
	runconfig *const cfg = allocforncores();

//	eprintf("alloc DONE\n"); fflush(stderr);

	configpagelen(cfg, argc, argv);

	cfg->nworkers = defnw;
	cfg->size = defsz;

	int i;

	if(argc > 1 && (i = atoi(argv[1])) > 0)
	{
		cfg->nworkers = i;
	}

	if(argc > 2 && (i = atoi(argv[2])) > 0)
	{
		cfg->size = i;
	}

	eprintf("configured"
		"\n\tworkers: %u"
		"\n\tsize: %u"
		"\n\tsz/wrk: %u"
		"\n\tflags: %u"
		"\n\tpagelength: %uKiB"
		"\n\tcorelist (%u):",
		cfg->nworkers, cfg->size, cfg->size/cfg->nworkers,
		cfg->flags, cfg->pagelength >> 10, cfg->ncores);

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
