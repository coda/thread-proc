#include <./worker.h>
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char** argv)
{
	fillconfig(argc, argv);

	fprintf(stderr, "nworkers: %u; niterations: %u; overall: %u\n",
		cfg.nworkers, cfg.niterations,
		cfg.nworkers * cfg.niterations);

	pid_t processes[cfg.nworkers];

	rnode* rings[nrings];
	bzero(rings, nrings * sizeof(rnode*));

	unsigned ok = 1;
	unsigned i;
	for(i = 0; ok && i < cfg.nworkers; i += 1)
	{
		pid_t p = fork();
		if(p == 0)
		{
			int rv = worker(rings, i);
			if(rv == 0) {} else
			{
				fprintf(stderr, "work %u failed\n", i);
			}

			exit(0);
		}
		if(p > 0)
		{
			processes[i] = p;
		}
		else
		{
			ok = 0;
		}
	}

	unsigned nactualwrk = i;
	fprintf(stderr, "actual workers: %u\n", nactualwrk);

	for(i = 0; i < nactualwrk; i += 1)
	{
		waitpid(processes[i], NULL, 0);
	}
	
	return 0;
}
