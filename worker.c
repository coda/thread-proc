#include <./worker.h>
#include <stdlib.h>
#include <stdio.h>

const unsigned nrings = 256;
configuration cfg;

void fillconfig(unsigned argc, char** argv)
{
	unsigned nw = 64;
	unsigned ni = 10000;

	if(argc > 1)
	{
		ni = atoi(argv[1]);
		ni = ni > 0 ? ni : 10000 * nw;
	}

	if(argc > 2)
	{
		nw = atoi(argv[2]);
		nw = nw > 0 ? nw : 64;
	}

	cfg.nworkers = nw;
	cfg.niterations = ni / nw;
}

static void freering(rnode* rn)
{
	if(rn != NULL)
	{
		rnode* t;
		rnode* r;
		for(r = rn; (t = r->next) != rn; r = t)
		{
//			fprintf(stderr, "freeing node %p\n", r);
			free(r);
		}
//		fprintf(stderr, "freeing node %p\n", r);
		free(r);
	}
}

void freerings(rnode* rings[])
{
	for(unsigned i = 0; i < nrings; i += 1)
	{
//		fprintf(stderr, "freeing ring %u\n", i);
		freering(rings[i]);
	}
}

static int walknode(rnode **const rp, unsigned n)
{
	rnode* r = *rp;
	
	if(r != NULL)
	{
		for(n /= 4; n > 0; n -= 1)
		{
			*(unsigned*)((unsigned char*)r + sizeof(rnode)) = n;
			r = r->next;
		}

		*rp = r;
	}

	return 0;
}

static int freenode(rnode **const rp, unsigned n)
{
	rnode* r = *rp;

//	fprintf(stderr, "freenode. r: %p\n", r);

	if(r != NULL)
	{
		if(r->next != r)
		{
			for(n /= 4; n > 0; n -= 1)
			{
				r = r->next;
			}

			r->prev->next = r->next;
			r->next->prev = r->prev;
			*rp = r->next;

			free(r);
		}
		else // ring is single element
		{
			*rp = NULL;
			free(r);
		}
	}

	return 0;
}

static void dumpring(rnode* rn)
{
	return; 

	if(rn != NULL)
	{
		rnode* r;
		for(r = rn; r->next != rn; r = r->next)
		{
			fprintf(stderr, "N. %p: %p - %p\n",
				r, r->prev, r->next);
		}
		fprintf(stderr, "N. %p: %p - %p\n", r, r->prev, r->next);

	}
	else
	{
		fprintf(stderr, "NULL\n");
	}
}

static int makenode(rnode **const rp, unsigned n)
{
	rnode* r = *rp;
	rnode* t = malloc(n * 64);
	int rv = 0;

	if(t != NULL)
	{
		if(r != NULL)
		{
			t->next = r;
			t->prev = r->prev;

			r->prev->next = t;
			r->prev = t;
		}
		else
		{
			t->next =
			t->prev = t;
			*rp = t;
		}
	}
	else
	{
		rv = 1;
	}

//	fprintf(stderr, "makenode. %d\n", rv);
	dumpring(*rp);

	return rv;
}

static int (*const operations[])(rnode**const, unsigned int) =
{
	walknode,
	makenode,
	freenode,
	walknode,
	makenode,
	freenode,
	walknode,
	makenode
};

int worker(rnode* rings[], unsigned id)
{
	unsigned int rseed = id;
	unsigned step = cfg.niterations;
	int rv = 0;

	while(rv == 0 && step)
	{
		unsigned op = rand_r(&rseed)
			% (sizeof(operations) / sizeof(void*));
		unsigned r = rand_r(&rseed) % nrings;
		unsigned param = 1 << (rand_r(&rseed) % 7 + 1);

//		fprintf(stderr, "work %u; op: %u; r: %u\n", id, op, r);

		rv = operations[op](rings + r, param);
		step -= 1;
	}

	return rv;
}
