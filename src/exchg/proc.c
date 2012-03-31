#include <exchg/heapsum.h>
#include <exchg/vector.h>
#include <exchg/ringlink.h>
#include <exchg/wrapper.h>
#include <util/spawn.h>
#include <util/config.h>
#include <util/tools.h>
#include <util/memmap.h>
#include <util/memfile.h>
#include <echotwo.h>

#include <unistd.h>
#include <sched.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <stdio.h>

// Definition of ring creation and operation routines

typedef struct
{
	int downread;
	int upwrite;
} pipelink;

static void pldrop(const pipelink *const pl)
{
	int t;

	if((t = pl->downread) != -1)
	{
		wprclose(t);
	}

	if((t = pl->upwrite) != -1)
	{
		wprclose(t);
	}
}

typedef struct
{
	const treeplugin * tp;
	unsigned id;

	pipelink links[2];
	int readend;
	int writeend;
} ringargument;

enum { right = 0, left }; // left child has 2 * id + 1 number

static void * makeargument(
	const treeplugin *const tp,
	const unsigned id, const void *const prevarg)
{
	const ringargument *const pra = (const ringargument *)prevarg;
	const runconfig *const rc = tp->rc;

	ringargument *const ra = malloc(sizeof(ringargument));
	if(ra) { } else
	{
		fail("can't allocate ring argument");
	}

	*ra = (ringargument){
		.tp = tp,
		.id = id,
		.links = { { -1, -1 }, { -1, -1 } },
		.readend = -1,
		.writeend = -1 };

	pipelink lnk;

	if(id == 0)
	{
		int fds[2];
		wprpipe(fds);
		lnk.downread = fds[piperead];
		lnk.upwrite = fds[pipewrite];
	}
	else
	{
		lnk = pra->links[id%2];

		wprclose(pra->readend);
		wprclose(pra->writeend);
		pldrop(pra->links + (id%2 ^ 1));
	}

	free((void *)pra);

	ra->readend = lnk.downread;

	if(2 * id + 1 >= rc->nworkers) // leaf node
	{
		ra->writeend = lnk.upwrite;
	}
	else
	{
		ra->links[left].upwrite = lnk.upwrite;

		int fds[2];
		wprpipe(fds);
		ra->writeend = fds[pipewrite];

		if(2 * id + 2 < rc->nworkers)
		{
			ra->links[right].downread = fds[piperead];

			wprpipe(fds);
			ra->links[left].downread = fds[piperead];
			ra->links[right].upwrite = fds[pipewrite];
		}
		else // no subtree to the right
		{
			ra->links[left].downread = fds[piperead];
		}
	}

	return ra;
}

#define actionfunction(fname) unsigned fname \
( \
	const runconfig *const rc, \
	ringlink *const rl, \
	vectorfile *const vf, vector *const v, \
	const unsigned id, \
	const unsigned r \
)

static actionfunction(expand);
static actionfunction(shrink);
static actionfunction(exchange);

static actionfunction((*const functions[])) =
{
	expand,
	shrink,
	exchange,
	expand,
	shrink,
	exchange,
	expand,
	shrink
};

const unsigned nfunctions = (sizeof(functions) / sizeof(void *));

static void treeroutine(const void *const arg)
{
	const ringargument *const ra = (const ringargument *)arg;
	ringlink rl;
	rlform(&rl, ra->readend, ra->writeend);

	const runconfig *const rc = ra->tp->rc;
	const unsigned jid = ra->id;
	elvector *const vcts = ra->tp->extra;

	pldrop(ra->links + 0);
	pldrop(ra->links + 1);
	free((void *)ra);

	vector v = {
		.ptr = peekmap(rc, -1, 0, rc->pagelength, pmwrite),
		.capacity = rc->pagelength,
		.length = 0,
		.offset = 0 };

	const unsigned iters = rc->size / rc->nworkers;
	unsigned id = jid;
	unsigned seed = jid;
	unsigned i;
	actionfunction((* lastfn)) = NULL;

	for(i = 0; id != (unsigned)-1 && i < iters; i += 1)
	{
		const unsigned r = rand_r(&seed);
		const unsigned fn = r % nfunctions;

		lastfn = functions[fn];
		id = lastfn(rc, &rl, &vcts[id].vf, &v, id, r);
	}

	if(lastfn != exchange)
	{
		vectorupload(rc, &v, &vcts[id].vf);
	}

	rlwrite(&rl, (unsigned)-1);

	printf("job %03u done. iters %u; exchanges %u. on core %d\n",
		jid, i, rl.nexchanges, sched_getcpu());		
}

int main(const int argc, const char *const argv[])
{
	const runconfig *const rc = formconfig(argc, argv, 64, 256 * 1024);
	elvector *const vcts
		= peekmap(rc, -1, 0, rc->nworkers * sizeof(elvector),
			pmshared | pmwrite);

	for(unsigned i = 0; i < rc->nworkers; i += 1)
	{
		vcts[i].vf = (vectorfile){
			.fd = makeshm(rc, 0), .length = 0, .offset = 0 };
	}

	fflush(stdout);

	disablesigpipe();

	const treeplugin tp = {
		.treeroutine = treeroutine,
		.makeargument = makeargument,
		.dropargument = NULL,
		.rc = rc,
		.extra = vcts };

	treespawn(&tp);

	printf("some values\n");
	for(unsigned i = 0; i < rc->nworkers; i += 1)
	{
		const unsigned len = vcts[i].vf.length / sizeof(eltype);
		if(len)
		{
			printf("%f\t%u of %u\n",
				(double)vfelat(&vcts[i].vf, len/2), len/2, len);
		}
		else
		{
			printf("EMPTY\n");
		}
	}

	for(unsigned i = 0; i < rc->nworkers; i += 1)
	{
		dropshm(vcts[i].vf.fd);
	}

	freeconfig((runconfig *)tp.rc);
	
	return 0;
}

// Definition of action functions

actionfunction(expand) // fn(rc, rl, vf, v, id, r) id
{
	const unsigned n = r % workfactor;
	unsigned seed = r;
	
	eltype *const buf = vectorexpand(rc, v, n);

	for(unsigned i = 0; i < n; i += 1)
	{
		buf[i] = elrand(&seed);
	}

	v->length += n * sizeof(eltype);

	return id;
}

static unsigned min(const unsigned a, const unsigned b)
{
	return a < b ? a : b;
}

actionfunction(shrink)
{
	const unsigned cnt = v->length / sizeof(eltype);
	const unsigned n = min(r % workfactor, cnt);

	if(n > 0)
	{
		eltype *const buf = vectorexpand(rc, v, 1);
		buf[0] = heapsum((eltype *)(v->ptr + v->offset), n);

		v->offset += n * sizeof(eltype);
		v->length -= (n - 1) * sizeof(eltype);
		vectorshrink(rc, v);
	}

	return id;
}

actionfunction(exchange)
{
	rl->nexchanges += 1;

	vectorupload(rc, v, vf);

	rlwrite(rl, id);
	const unsigned i = rlread(rl);

	if(i != (unsigned)-1)
	{
		vectorfile *const ivf = &(((elvector *)vf) - id + i)->vf;
		vectordownload(rc, ivf, v);
	}

	return i;
}
