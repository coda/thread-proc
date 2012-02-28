#include <util/config.h>
#include <util/echotwo.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sched.h>

static runconfig * allocforncores(void)
{
	const unsigned ncoresmax = 128;
	const unsigned cslen = CPU_ALLOC_SIZE(ncoresmax);
	printf("assuming no more than %u cores. set length = %u\n",
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
			cfg->corelist[cc] = i;
			cc += 1;
		}
	}

	free(coreset);

	return cfg;
}

static unsigned ispowerof2(unsigned x)
{
	return (x ^ (x - 1)) + 1 == x << 1;
}

static void configpagelen(runconfig *const cfg, const unsigned arg)
{
	const long sysplen = sysconf(_SC_PAGESIZE);
	if(sysplen != -1) { } else
	{
		fail("can't get system pagesize");
	}

	if(sysplen < 0x7fffffff) { } else
	{
		fail("system page length %ld is too long", sysplen);
	}

	unsigned flags = 0;
	unsigned plen = sysplen;

	const long alen = arg * 1024;
	if(alen < 0x7fffffff) { } else
	{
		fail("argument page length %ld is too long", alen);
	}

	if(alen > 0)
	{
		if(ispowerof2(alen) && alen > sysplen)
		{
			plen = alen;
			flags |= cfghugetlb;
		}
		else
		{
			fail("%u don't look like huge page length in KiB", arg);
		}
	}

	cfg->flags |= flags;
	cfg->pagelength = plen;
}

static const char * shift(const char *const * * pargv)
{
	*pargv += 1;
	return **pargv;
}

static const char *const usage =
	"usage: [-n N] [-s N] [-p N] [-a I|G]\n"
		"\t-n N\tnum of workers\n"
		"\t-s N\twork size\n"
		"\t-p N\tpage length\n"
		"\t-a I|G\taffinity type";

static unsigned readintarg(const char *const * * pargv)
{
	unsigned n = 0;
	const char *const arg = shift(pargv);
	if(arg && sscanf(arg, "%u", &n) == 1 && n > 0) { } else
	{
		fail("can't parse '%s' as int. %s", arg, usage);
	}

	return n;
}

runconfig * formconfig(
	const int argc, const char *const * argv,
	const unsigned defnw, const unsigned defsz)
{
	runconfig *const cfg = allocforncores();

	configpagelen(cfg, 0);

	cfg->nworkers = defnw;
	cfg->size = defsz;
	cfg->flags = 0;

	struct
	{
		unsigned nset:1;
		unsigned sset:1;
		unsigned pset:1;
		unsigned aset:1;
	} flags = { 0, 0, 0, 0 };

	const char * arg = shift(&argv);
	while(arg)
	{
		if(!strcmp(arg, "-n"))
		{
			if(!flags.nset)
			{
				cfg->nworkers = readintarg(&argv);
				flags.nset = 1;
			}
			else
			{
				fail("-n already specified. %s", usage);
			}
		}
		else if(!strcmp(arg, "-s"))
		{
			if(!flags.sset)
			{
				cfg->size = readintarg(&argv);
				flags.sset = 1;
			}
			else
			{
				fail("-s already specified. %s", usage);
			}
		}
		else if(!strcmp(arg, "-p"))
		{
			if(!flags.pset)
			{
				configpagelen(cfg, readintarg(&argv));
				flags.pset = 1;
			}
			else
			{
				fail("-p already specified. %s", usage);
			}
		}
		else if(!strcmp(arg, "-a"))
		{
			if(flags.aset)
			{
				fail("-a already specified. %s", usage);
			}
			else
			{
				flags.aset = 1;

				arg = shift(&argv);

				if(strcmp(arg, "I") == 0)
				{
					cfg->flags |= cfgaffinity;
				}
				else if(strcmp(arg, "G") == 0)
				{
					cfg->flags |= cfgaffinity;
					cfg->flags |= cfgaffinegroup;
				}
				else
				{
					fail("unknown affinity grouping '%s'. "
						"%s", arg, usage);
				}
			}
		}
		else
		{
			fail(usage, "unknown option");
		}

		arg = shift(&argv);
	}

	printf("configured"
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
		printf(" %u", cfg->corelist[i]);
	}

	printf("\n");

	fflush(stdout);

	return cfg;
}

void freeconfig(runconfig *const rc)
{
	if(rc)
	{
		free(rc);
	}
}
