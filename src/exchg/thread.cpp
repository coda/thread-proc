extern "C" {
#include <exchg/heapsum.h>
#include <exchg/ringlink.h>
#include <exchg/wrapper.h>
#include <util/spawn.h>
#include <util/echotwo.h>
#include <util/config.h>
#include <util/tools.h>
#include <unistd.h>
#include <sched.h>
#include <stdlib.h>
#include <stdio.h>
}

#include <vector>
#include <algorithm>

typedef struct
{
	int downread;
	int upwrite;
} pipelink;

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

	ringargument *const ra = (ringargument *)malloc(sizeof(ringargument));
	if(ra) { } else
	{
		fail("can't allocate ring argument");
	}

	ra->tp = tp;
	ra->id = id;

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
	}

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

static void dropargument(void *const arg)
{
	const ringargument *const ra = (const ringargument *)arg;

	wprclose(ra->readend);
	wprclose(ra->writeend);
	free(arg);
}

using namespace std;

struct elvector
{
	vector<eltype> v;
	unsigned char padding[defpad(sizeof(v), cachelinelength)];
};

static unsigned expand(
	vector<eltype>&, ringlink *const, const unsigned, const unsigned);

static unsigned shrink(
	vector<eltype>&, ringlink *const, const unsigned, const unsigned);

static unsigned exchange(
	vector<eltype>&, ringlink *const, const unsigned, const unsigned);

static unsigned (*const functions[])(
	vector<eltype>&, ringlink *const, const unsigned, const unsigned) =
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

static const unsigned nfunctions = sizeof(functions) / sizeof(void *);

static void treeroutine(const void *const arg)
{
	const ringargument *const ra = (const ringargument *)arg;
	const runconfig *const rc = ra->tp->rc;
	elvector *const vcts = (elvector *)ra->tp->extra;

	ringlink rl;
	rlform(&rl, ra->readend, ra->writeend);

	const unsigned iters = rc->size / rc->nworkers;

	unsigned id = ra->id;
	unsigned seed = id;

	unsigned previd;
	unsigned i = 0;

	try
	{
		for(i = 0; id != (unsigned)-1 && i < iters; i += 1)
		{
			const unsigned r = rand_r(&seed);
			const unsigned fn = r % nfunctions;

			previd = id;
			id = functions[fn](vcts[previd].v, &rl, id, r);
		}
	}
	catch(const exception& e)
	{
		fail("job %03u exception: %s", e.what());
	}

	rlwrite(&rl, (unsigned)-1);

	printf("job %03u done. iters %u; exchanges %u. on core %d\n",
		ra->id, i, rl.nexchanges, sched_getcpu());
}

static void process(const runconfig *const rc)
{
	elvector *const arrays = new elvector[rc->nworkers];

	const treeplugin tp = {
		makeargument,
		dropargument,
		treeroutine,
		rc,
		arrays };

	treespawn(&tp);

	printf("some values\n");
	for(unsigned i = 0; i < rc->nworkers; i += 1)
	{
		const unsigned long len = arrays[i].v.size();
		if(len)
		{
			printf("%f\t%lu of %lu\n",
				(double)arrays[i].v[len/2], len/2, len);
		}
		else
		{
			printf("\tEMPTY\n");
		}
	}
	
	delete[] arrays;
}

int main(const int argc, const char *const argv[])
{
	disablesigpipe();

	const runconfig *const rc = formconfig(argc, argv, 64, 256 * 1024);

	try
	{
		process(rc);
	}
	catch(const exception& e)
	{
		fail("something wrong. %s\n", e.what());
	}

	freeconfig((runconfig *)rc);
	
	return 0;
}


static unsigned expand(
	vector<eltype>& vct, ringlink *const rl,
	const unsigned id, const unsigned r)
{
	const unsigned n = r % workfactor;
	unsigned seed = r;

	vct.reserve(vct.size() + n);
	for(unsigned i = 0; i < n; i += 1)
	{
		vct.push_back(elrand(&seed));
	}

	return id;
}

static unsigned shrink(
	vector<eltype>& vct, ringlink *const rl,
	const unsigned id, const unsigned r)
{
 	const unsigned n = min(
		(unsigned long)(r % workfactor), (unsigned long)vct.size());
 
	if(n > 0)
	{
		const eltype sum = heapsum(&vct[0], n);
		vct.push_back(sum);

 		vector<eltype>::iterator b = vct.begin();
 		vct.erase(b, b + n);
	}

	return id;
}

static unsigned exchange(
	vector<eltype>& vct, ringlink *const rl,
	const unsigned id, const unsigned n)
{
	rl->nexchanges += 1;
	rlwrite(rl, id);
	return rlread(rl);
}
