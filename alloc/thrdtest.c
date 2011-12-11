#include <./worker.h>
#include <stdio.h>
#include <stdint.h>
#include <strings.h>
#include <pthread.h>

static void* routine(void* arg)
{
	unsigned id = (uintptr_t)arg;

	rnodeline rings[nrings];
	bzero(rings, nrings * sizeof(rnodeline));

	int rv = worker(rings, id);
	if(rv == 0)
	{
		fprintf(stderr, "work %u done\n", id);
	}
	else
	{
		fprintf(stderr, "work %u failed\n", id);
	}
	freerings(rings);

	return NULL;
}

int main(int argc, char** argv)
{
	fillconfig(argc, argv);
	pthread_t threads[cfg.nworkers];

	fprintf(stderr, "nworkers: %u; niterations: %u; overall: %u\n",
		cfg.nworkers, cfg.niterations,
		cfg.nworkers * cfg.niterations);

	unsigned ok = 1;
	unsigned i;
	for(i = 0; ok && i < cfg.nworkers; i += 1)
	{
		ok = pthread_create(threads + i, NULL, routine, (void*)i) == 0;
	}

	unsigned nactualwrks = i;

	fprintf(stderr, "actual workers: %u\n", nactualwrks);

	for(i = 0; i < nactualwrks; i += 1)
	{
		pthread_join(threads[i], NULL);
	}
	
	return 0;
}
