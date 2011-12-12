#include <./util.h>
#include <./work.h>

#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>

typedef struct
{
	int fd;
	unsigned length; // overall length;
	unsigned eloffset;
	eltype * elements;
	unsigned elcount;
} vector;

static eltype * vels(const vector *const v)
{
	return v->elements + v->eloffset;
}

typedef struct
{
	vector v;
	unsigned char padding[defpad(sizeof(vector), cachelinelength)];
} elvector;

static unsigned expand(
	vector *const vector, ringlink *const,
	const unsigned id, const unsigned n);

static unsigned shrink(
	vector *const, ringlink *const,
	const unsigned, const unsigned);

static unsigned exchange(
	vector *const, ringlink *const,
	const unsigned, const unsigned);

static unsigned (*const functions[])(
	vector *const, ringlink *const, const unsigned,
	const unsigned) =
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

static void routine(
	ringlink *const rl, elvector *const vectors,
	const testconfig *const cfg, const unsigned jid)
{
	const unsigned iters = cfg->niterations / cfg->nworkers;

	unsigned seed = jid;
	unsigned id = jid;
	unsigned i;
	for(i = 0; id != (unsigned)-1 && i < iters; i += 1)
	{
			const unsigned r = rand_r(&seed);
			const unsigned fn = r % nfunctions;

			id = functions[fn](&vectors[id].v, rl, id, r);
	}

	uiwrite(rl->towrite, (unsigned)-1);

 	printf("unit %03u(%u) done. iters: %u; exchanges: %u\n",
 		jid, getpid(), i, rl->nexchanges);

	sleep(100000);
}

static void runjobs(
	const testconfig *const cfg,
	elvector *const vectors,
	void (*const code)(
		ringlink *const, elvector *const, const testconfig *const,
		unsigned))
{
	const unsigned count = cfg->nworkers;
	pid_t procs[count];

	unsigned ok = 1;
	unsigned err = 0;

	int p0rd = -1;
//	int p0wr = -1;
	int p1rd = -1;
	int p1wr = -1;

//	int zerord = -1;
	int zerowr = -1;

	unsigned i;

	// explicit external ifs would be too complex (if(count > 0) and so on).
	// Compiler should infer them during optimization
	for(i = 0; ok && i < count; i += 1)
	{
		if(i != 0)
		{
			p0rd = p1rd;
//			p0wr = p1wr;
		}
		else
		{
// 			makerlink(&p0wr, &p0rd);
// 			zerord = p0rd;
// 			zerowr = p0wr;

			makerlink(&zerowr, &p0rd);
		}

		if(i != count - 1)
		{
			makerlink(&p1wr, &p1rd);
		}
		else
		{
			p1wr = zerowr;
//			p1rd = zerord;
		}

		pid_t p = fork();
		if(p == 0)
		{
			ringlink rl = {
				.toread = p0rd,
				.towrite = p1wr,
				.nexchanges = 0,
				.writable = 1 };
// 			rl.toread = p0rd;
// 			rl.towrite = p1wr;

// 			eprintf(
// 				"%03u p0: (%d; %d); p1: (%d; %d); "
// 				"need: (%d; %d); close: (%d; %d)\n",
// 				i, p0rd, p0wr, p1rd, p1wr,
// 				p0rd, p1wr, p1rd, p0wr);

// 			if(i != count - 1)
// 			{
// 				uclose(p0wr);
// 				uclose(p1rd);
// 			}

			eprintf("%03u. need: %d %d; ", i, p0rd, p1wr);

			if(i != count - 1)
			{
				eprintf("close: %d; ", zerowr); 
				uclose(zerowr);

// 				if(i)
// 				{
// 					eprintf("close %d", p1rd);
// 					uclose(p1rd);
// 				}

				eprintf("close %d", p1rd);
				uclose(p1rd);
			}

			eprintf("\n");

			code(&rl, vectors, cfg, i);
			exit(0);
		}
		else if(p > 0)
		{
			procs[i] = p;

			uclose(p0rd);
			uclose(p1wr);

// 			if(i != count - 1) {} else
// 			{
// 				uclose(zerowr);
// 			}
		}
		else
		{
			ok = 0;
		}

		ok = err == 0;
	}

	if(ok) {} else
	{
		fail("can't start %u jobs", count);
	}

	printf("%u jobs stated\n", i);

	for(i = 0; ok && i < count; i += 1)
	{
		const pid_t p = waitpid(procs[i], NULL, 0);
		ok = p == procs[i];
	}

	if(ok) {} else
	{
		fail("can't join %u jobs", count);
	}

	printf("%u jobs joined\n", i); 
}

int main(const int argc, const char *const *const argv)
{
	ignoresigpipe();

	const testconfig cfg = fillconfig(argc, argv);
	const unsigned vectslen = sizeof(elvector) * cfg.nworkers;
	
	elvector *const vectors
		= (elvector *)peekmap(&cfg, -1, 0, vectslen, PROT_WRITE);

	fflush(stderr);
	fflush(stdout);
	runjobs(&cfg, vectors, routine);

	printf("some values\n");

	for(unsigned i = 0; i < cfg.nworkers; i += 1)
	{
		if(vectors[i].v.elcount)
		{
			printf("\t%f\n", vels(&vectors[i].v)[0]);
		}
		else
		{
			printf("\tEMPTY\n");
		}
	}

	printf("DONE. vector size: %lu; elvector size %lu\n",
		(unsigned long)sizeof(vector),
		(unsigned long)sizeof(elvector));	

	dropmap(&cfg, vectors, vectslen);

	return 0;
}

static unsigned expand(
	vector *const v, ringlink *const rl,
	const unsigned id, const unsigned r)
{
// 	const unsigned n = r % workfactor;
// 	unsigned seed = r;
// 
// 	array.reserve(array.size() + n);
// 	for(unsigned i = 0; i < n; i += 1)
// 	{
// 		array.push_back(elrand(&seed));
// 	}

	return id;
}

static unsigned shrink(
	vector *const v, ringlink *const rl,
	const unsigned id, const unsigned r)
{
// 	const unsigned n = min((unsigned long)(r % workfactor), array.size());
// 
// 	if(n > 0)
// 	{
// 		array.push_back(heapsum(&array[0], n));
// 
// 		vector<eltype>::iterator b = array.begin();
// 		array.erase(b, b + n);
// 	}

	return id;
}

static unsigned exchange(
	vector *const v, ringlink *const rl,
	const unsigned id, const unsigned n)
{
	rl->nexchanges += 1;

	if(rl->writable)
	{
		rl->writable = uiwrite(rl->towrite, id);
	}

	return uiread(rl->toread);
}
