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
	unsigned pagelength;

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

static char * peekmap(
	const int fd,
	const unsigned offset,
	const unsigned len,
	const unsigned flag)
{
	void * m = mmap(NULL, len, PROT_READ | flag, MAP_SHARED, fd, offset);
	if(m != MAP_FAILED) {} else
	{
		eprintf("err: %s. pid: %u. can't peek. len: %u; off: %u\n",
			strerror(errno), getpid(), len, offset);
		exit(-1);
	}

//	eprintf("peekmap done\n");

	return m;
}

static void randroutine(const unsigned id)
{
	const unsigned sz = setup.cfg.size;
	const unsigned nwrks = setup.cfg.nworkers; 
	const unsigned plen = setup.pagelength;

	const unsigned m = sz;
	const unsigned n = sz;

	const jobitem ji = ballance(id, nwrks, sz);
	const unsigned l = ji.nrows;
	const unsigned sr = ji.startrow;
	
	const unsigned aoff = sr * m * sizeof(eltype);
	const unsigned alen = l * m * sizeof(eltype);

	// and for b because matricies are square
	const unsigned boff = sr * n * sizeof(eltype);
	const unsigned blen = l * n * sizeof(eltype);

	// recalculate offset and length in pages
	const unsigned amapoff = aligndown(aoff, plen);
	const unsigned amapdiff = aoff - amapoff;
	const unsigned amaplen = align(alen + amapdiff, plen);

	const unsigned bmapoff = aligndown(boff, plen);
	const unsigned bmapdiff = boff - bmapoff;
	const unsigned bmaplen = align(blen + bmapdiff, plen);

// 	eprintf("pid %u. peeking a. off: %u; len: %u\n",
// 		getpid(), amapoff, amaplen);

	eltype *const a = (eltype *const)(
		peekmap(setup.fda, amapoff, amaplen, PROT_WRITE) + amapdiff);

// 	eprintf("pid %u. peeking b. off: %u; len: %u\n",
// 		getpid(), bmapoff, bmaplen);

	eltype *const b = (eltype *const)(
		peekmap(setup.fdb, bmapoff, bmaplen, PROT_WRITE) + bmapdiff);

	matrand(id, a, l, m);
	matrand(id * 5, b, l, n);

	printf("rand %03u with %u rows is done\n", id, l);
}

static void multroutine(const unsigned id)
{
	const unsigned sz = setup.cfg.size;
	const unsigned nwrks = setup.cfg.nworkers; 
	const unsigned plen = setup.pagelength;

	const unsigned m = sz;
	const unsigned n = sz;

	const jobitem ji = ballance(id, nwrks, sz);
	const unsigned l = ji.nrows;
	const unsigned sr = ji.startrow;
	
	const unsigned aoff = sr * m * sizeof(eltype);
	const unsigned alen = l * m * sizeof(eltype);

	const unsigned amapoff = aligndown(aoff, plen);
	const unsigned amapdiff = aoff - amapoff;
	const unsigned amaplen = align(alen + amapdiff, plen);

	const unsigned bmapoff = 0;
	const unsigned bmaplen = align(m * n * sizeof(eltype), plen);

	const unsigned roff = sr * n * sizeof(eltype);
	const unsigned rlen = l * n * sizeof(eltype);

	const unsigned rmapoff = aligndown(roff, plen);
	const unsigned rmapdiff = roff - rmapoff;
	const unsigned rmaplen = align(rlen + rmapdiff, plen);

	const eltype *const a = (const eltype *const)(
		peekmap(setup.fda, amapoff, amaplen, 0) + amapdiff);

	const eltype *const b = (const eltype *const)(
		peekmap(setup.fdb, bmapoff, bmaplen, 0));

	eltype *const r = (eltype *const)(
		peekmap(setup.fdr, rmapoff, rmaplen, PROT_WRITE) + rmapdiff);
	
	matmul(a, b, l, m, n, r);

	printf("mult %u with %u rows is done\n", id, l);
}

int main(int argc, const char *const *const argv)
{
	setup.cfg = fillconfig(argc, argv);
	const unsigned sz = setup.cfg.size;
	const unsigned nw = setup.cfg.nworkers;

	printf("nworkers: %u; matrix size: %fMiB\n",
		nw, (double)sz * sz * sizeof(eltype) / (double)(1 << 20));

	setup.fda = makeshm(sz * sz * sizeof(eltype));
	setup.fdb = makeshm(sz * sz * sizeof(eltype));
	setup.fdr = makeshm(sz * sz * sizeof(eltype));
	setup.pagelength = getpagelength();

	printf("shm allocated\n");

	printf("randomization\n"); fflush(stdout); fflush(stderr);
	runjobs(nw, randroutine);

	printf("multiplication\n"); fflush(stdout); fflush(stderr);
	runjobs(nw, multroutine);

	return 0;
}
