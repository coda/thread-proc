#include <./alloc/worker.h>
#include <./util/echotwo.h>

#include <stdlib.h>
#include <stdio.h>

const unsigned nrings = NRINGS;
static const unsigned stepfactor = 1;

static void freering(rnode *const rn)
{
	if(rn != NULL)
	{
		rnode * t;
		rnode * r;
		for(r = rn; (t = r->next) != rn; r = t)
		{
			free(r);
		}
		free(r);
	}
}

void freerings(rnode * rings[])
{
	for(unsigned i = 0; i < nrings; i += 1)
	{
		freering(rings[i]);
	}
}

static void walknode(rnode **const rp, unsigned n)
{
	rnode* r = *rp;
	
	if(r != NULL)
	{
		for(n /= stepfactor; n > 0; n -= 1)
		{
			*(unsigned*)((unsigned char*)r + sizeof(rnode)) = n;
			r = r->next;
		}

		*rp = r;
	}
}

static void freenode(rnode **const rp, unsigned n)
{
	rnode* r = *rp;

	if(r != NULL)
	{
		if(r->next != r)
		{
			for(n /= stepfactor; n > 0; n -= 1)
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
}

static void dumpring(rnode* rn)
{
	return; 

	if(rn != NULL)
	{
		rnode* r;
		for(r = rn; r->next != rn; r = r->next)
		{
			eprintf("N. %p: %p - %p\n",
				(void *)r, (void *)r->prev, (void *)r->next);
		}
		eprintf("N. %p: %p - %p\n",
			(void *)r, (void *)r->prev, (void *)r->next);

	}
	else
	{
		eprintf("NULL\n");
	}
}

static void makenode(rnode **const rp, unsigned n)
{
	rnode* r = *rp;
	rnode* t = malloc(n * 64);

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
		fail("can't make node");
	}

	dumpring(*rp);
}

static void (*const operations[])(rnode **const, unsigned int) =
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

void worker(const runconfig *const rc, rnode * rings[], const unsigned id)
{
	unsigned int rseed = id;
	
	for(unsigned step = rc->size / rc->nworkers; step; step -= 1)
	{
		const unsigned op =
			rand_r(&rseed) % (sizeof(operations) / sizeof(void*));

		const unsigned r = rand_r(&rseed) % nrings;
		const unsigned param = 1 << (rand_r(&rseed) % 7 + 1);

		operations[op](&rings[r], param);
	}
}
