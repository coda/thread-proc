#include <./mul.h>
#include <./util.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <errno.h>
#include <unistd.h>

#include <sys/wait.h>
#include <sys/mman.h>

static struct
{
 	int fda;
 	int fdb;
 	int fdr;
//	unsigned pagelength;

 	testconfig cfg;
} setup;


static void runjobs(const unsigned count, void (* routine)(const unsigned))
{
	pid_t procs[count];
	unsigned ok = 1;
	
	for(unsigned i = 0; ok && i < count; i += 1)
	{
		pid_t p = fork();
		if(p == 0)
		{
			routine(i);
			exit(0);
		}
		if(p > 0)
		{
			procs[i] = p;
		}
		else
		{
			ok = 0;
		}
	}

	if(ok) {} else
	{
		eprintf("err: %s. can't start %u jobs\n",
			strerror(errno), count);
		exit(1);
	}

	for(unsigned i = 0; ok && i < count; i += 1)
	{
		ok = waitpid(procs[i], NULL, 0) == procs[i];
	}

	if(ok) {} else
	{
		eprintf("err: %s. can't join %u jobs\n",
			strerror(errno), count);
		exit(1);
	}
}

// static char * peekmap(
// 	const int fd,
// 	const unsigned offset,
// 	const unsigned len,
// 	const unsigned flag)
// {
// 	void * m = mmap(NULL, len, PROT_READ | flag, MAP_SHARED, fd, offset);
// 	if(m != MAP_FAILED) {} else
// 	{
// 		eprintf("err: %s. pid: %u. can't peek. len: %u; off: %u\n",
// 			strerror(errno), getpid(), len, offset);
// 		exit(-1);
// 	}
// 
// //	eprintf("peekmap done\n");
// 
// 	return m;
// }

static void randroutine(const unsigned id)
{
	const unsigned sz = setup.cfg.size;
// 	const unsigned nwrks = setup.cfg.nworkers; 
// 	const unsigned plen = setup.cfg.pagelength;

	const unsigned l = sz;
	const unsigned m = sz;
	const unsigned n = sz;

// 	const jobitem ji = ballance(id, nwrks, sz);
// 	const unsigned l = ji.nrows;
// 	const unsigned sr = ji.startrow;
// 	
// 	const unsigned aoff = sr * m * sizeof(eltype);
// 	const unsigned alen = l * m * sizeof(eltype);
// 
// 	// and for b because matricies are square
// 	const unsigned boff = sr * n * sizeof(eltype);
// 	const unsigned blen = l * n * sizeof(eltype);
// 
// 	// recalculate offset and length in pages
// 	const unsigned amapoff = aligndown(aoff, plen);
// 	const unsigned amapdiff = aoff - amapoff;
// 	const unsigned amaplen = align(alen + amapdiff, plen);
// 
// 	const unsigned bmapoff = aligndown(boff, plen);
// 	const unsigned bmapdiff = boff - bmapoff;
// 	const unsigned bmaplen = align(blen + bmapdiff, plen);
// 
// // 	eprintf("pid %u. peeking a. off: %u; len: %u\n",
// // 		getpid(), amapoff, amaplen);
// 
// 	eltype *const a = (eltype *const)(peekmap(&setup.cfg,
// 		setup.fda, amapoff, amaplen, PROT_WRITE) + amapdiff);
// 
// // 	eprintf("pid %u. peeking b. off: %u; len: %u\n",
// // 		getpid(), bmapoff, bmaplen);
// 
// 	eltype *const b = (eltype *const)(peekmap(&setup.cfg,
// 		setup.fdb, bmapoff, bmaplen, PROT_WRITE) + bmapdiff);
// 
// 	matrand(id, a, l, m, tilecols);
// 	matrand(id * 5, b, l, n, tilerows);

	const joblayout al = definejob(&setup.cfg, id, l, m, tilecols);
	const joblayout bl = definejob(&setup.cfg, id, m, n, tilerows);

	const unsigned adiff = al.baseoffset - al.mapoffset;
	eltype *const a = (eltype *const)(peekmap(&setup.cfg,
		setup.fda, al.mapoffset, al.maplength, PROT_WRITE) + adiff);

	const unsigned bdiff = bl.baseoffset - bl.mapoffset;
	eltype *const b = (eltype *const)(peekmap(&setup.cfg,
		setup.fdb, bl.mapoffset, bl.maplength, PROT_WRITE) + bdiff);

	matrand(id, a, al.baserow, al.nrows, m, tilecols);
	matrand(id * 5, b, bl.baserow, bl.nrows, n, tilerows);

	printf("rand %03u with %u rows is done\n", id, l);
}

static void multroutine(const unsigned id)
{
	const unsigned sz = setup.cfg.size;
// 	const unsigned nwrks = setup.cfg.nworkers; 
//  	const unsigned plen = setup.cfg.pagelength;

	const unsigned l = sz;
	const unsigned m = sz;
	const unsigned n = sz;

// 	const jobitem ji = ballance(id, nwrks, sz);
// 	const unsigned l = ji.nrows;
// 	const unsigned sr = ji.startrow;
// 	
// 	const unsigned aoff = sr * m * sizeof(eltype);
// 	const unsigned alen = l * m * sizeof(eltype);
// 
// 	const unsigned amapoff = aligndown(aoff, plen);
// 	const unsigned amapdiff = aoff - amapoff;
// 	const unsigned amaplen = align(alen + amapdiff, plen);
// 
// 	const unsigned bmapoff = 0;
// 	const unsigned bmaplen = align(m * n * sizeof(eltype), plen);
// 
// 	const unsigned roff = sr * n * sizeof(eltype);
// 	const unsigned rlen = l * n * sizeof(eltype);
// 
// 	const unsigned rmapoff = aligndown(roff, plen);
// 	const unsigned rmapdiff = roff - rmapoff;
// 	const unsigned rmaplen = align(rlen + rmapdiff, plen);

	const joblayout al = definejob(&setup.cfg, id, l, m, tilecols);
	const unsigned adiff = al.baseoffset - al.mapoffset;

	const joblayout bl = definejob(
		&(testconfig){
			.nworkers = 1,
			.size = setup.cfg.size,
			.pagelength = setup.cfg.pagelength },
		0, m, n, tilerows);

	const joblayout rl = definejob(&setup.cfg, id, l, n, tilerows);
	const unsigned rdiff = rl.baseoffset - rl.mapoffset;

// 	const eltype *const a = (const eltype *const)(peekmap(&setup.cfg,
// 		setup.fda, amapoff, amaplen, 0) + amapdiff);
// 
// 	const eltype *const b = (const eltype *const)(peekmap(&setup.cfg,
// 		setup.fdb, bmapoff, bmaplen, 0));
// 
// 	eltype *const r = (eltype *const)(peekmap(&setup.cfg,
// 		setup.fdr, rmapoff, rmaplen, PROT_WRITE) + rmapdiff);

	const eltype *const a = (const eltype *const)(peekmap(&setup.cfg,
		setup.fda, al.mapoffset, al.maplength, 0) + adiff);

	const eltype *const b = (const eltype *const)(peekmap(&setup.cfg,
		setup.fdb, bl.mapoffset, bl.maplength, 0));

	eltype *const r = (eltype *const)(peekmap(&setup.cfg,
		setup.fdr, rl.mapoffset, rl.maplength, PROT_WRITE) + rdiff);
	
	matmul(a, b, al.baserow, al.nrows, m, n, r);

	printf("mult %u with %u rows is done\n", id, l);
}

int main(int argc, const char *const *const argv)
{
	setup.cfg = fillconfig(argc, argv);
	const unsigned sz = setup.cfg.size;
	const unsigned nw = setup.cfg.nworkers;

	printf("nworkers: %u; matrix size: %fMiB\n",
		nw, (double)sz * sz * sizeof(eltype) / (double)(1 << 20));

	setup.fda = makeshm(&setup.cfg, sz * sz * sizeof(eltype));
	setup.fdb = makeshm(&setup.cfg, sz * sz * sizeof(eltype));
	setup.fdr = makeshm(&setup.cfg, sz * sz * sizeof(eltype));
//	setup.pagelength = getpagelength();

	printf("shm allocated\n");

	printf("randomization\n"); fflush(stdout); fflush(stderr);
	runjobs(nw, randroutine);

	printf("multiplication\n"); fflush(stdout); fflush(stderr);
	runjobs(nw, multroutine);

	return 0;
}
