#include <matmul/mul.h>
#include <matmul/util.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <errno.h>
#include <unistd.h>

#include <sys/wait.h>
#include <sys/mman.h>
#include <linux/mman.h>
#include <sys/shm.h>

static struct
{
	eltype * a;
	eltype * b;
	eltype * r;

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
		int status;
		pid_t p = waitpid(procs[i], &status, 0);
		ok = p == procs[i]
			&& WIFEXITED(status)
			&& WEXITSTATUS(status) == 0;
	}

	if(ok) {} else
	{
		eprintf("err: %s. can't join %u jobs\n",
			strerror(errno), count);
		exit(1);
	}
}

static void randroutine(const unsigned id)
{
	const unsigned sz = setup.cfg.size;
//	const unsigned nwrks = setup.cfg.nworkers; 

	const unsigned l = sz;
	const unsigned m = sz;
	const unsigned n = sz;

// 	const jobitem ji = ballance(id, nwrks, sz);
// 	const unsigned l = ji.nrows;
// //	const unsigned sr = ji.startrow;
// 
// 	const unsigned sr = aligndown(ji.startrow, tilerows);
// 	const unsigned baserow = ji.startrow - sr;
// 	
// 	eltype *const a = setup.a + sr * m;
// 	eltype *const b = setup.b + sr * n;
// 
// 	matrand(id, a, baserow, l, m, tilecols);
// 	matrand(id * 5, b, baserow, l, n, tilerows);

	const unsigned tr = tilerows;
	const unsigned tc = tilecols;

	const joblayout al = definejob(&setup.cfg, id, l, m, tr, tc);
	const joblayout bl = definejob(&setup.cfg, id, m, n, tc, tr);

	eltype *const a = setup.a + al.baseoffset / sizeof(eltype);
	eltype *const b = setup.b + bl.baseoffset / sizeof(eltype);

	matrand(id, a, al.baserow, al.nrows, m, tilecols);
	matrand(id * 5, b, bl.baserow, bl.nrows, n, tilerows);

	printf("rand %u with %u rows is done\n", id, l);
}

static void multroutine(const unsigned id)
{
	const unsigned sz = setup.cfg.size;
//	const unsigned nwrks = setup.cfg.nworkers; 

	const unsigned l = sz;
	const unsigned m = sz;
	const unsigned n = sz;

// 	const jobitem ji = ballance(id, nwrks, sz);
// 	const unsigned l = ji.nrows;
// //	const unsigned sr = ji.startrow;
// 
// 	const unsigned sr = aligndown(ji.startrow, tilerows);
// 	const unsigned baserow = ji.startrow - sr;
// 	
// 	const eltype *const a = setup.a + sr * m;
// 	eltype *const r = setup.r + sr * n;
// 
// 	matmul(a, setup.b, baserow, l, m, n, r);

	const unsigned tr = tilerows;
	const unsigned tc = tilecols;

	const joblayout al = definejob(&setup.cfg, id, l, m, tr, tc);
	const joblayout rl = definejob(&setup.cfg, id, l, n, tr, tr);

	const eltype *const a = setup.a + al.baseoffset / sizeof(eltype);
	eltype *const r = setup.r + rl.baseoffset / sizeof(eltype);
	
	matmul(a, setup.b, al.baserow, al.nrows, m, n, r);

	printf("mult %u with %u rows is done\n", id, l);
}

// static eltype * matalloc(const unsigned m, const unsigned n)
// {
// 	const long plen = sysconf(_SC_PAGESIZE);
// 	if(plen > 0 && plen < (1 << 30)) {} else
// 	{
// 		eprintf("err: %s. can't get sane page size. plen: %ld\n",
// 			strerror(errno), plen);
// 		exit(1);
// 	}
// 
// 	const unsigned len = align(m * n * sizeof(eltype), plen);
// 
// 	printf("mmaping for len %u\n", len);
// 
// 	void * ptr = mmap(NULL, len, PROT_WRITE | PROT_READ, 
// 		MAP_ANONYMOUS | MAP_HUGETLB | MAP_SHARED,
// 		0, 0);
// 
// 	if(ptr != MAP_FAILED) {} else
// 	{
// 		eprintf("err: %s. can't allocate %u bytes\n", len);
// 		exit(1);
// 	}
// 	
// 	return ptr;
// }

static eltype * matalloc(const unsigned m, const unsigned n)
{
	return (eltype *)peekmap(&setup.cfg, -1, 0,
		m * n * sizeof(eltype),
		PROT_WRITE);
}

// static void * matalloc(const unsigned m, const unsigned n)
// {
// 	const long plen = sysconf(_SC_PAGESIZE);
// 	if(plen > 0 && plen < (1 << 30)) {} else
// 	{
// 		eprintf("err: %s. can't get sane page size. plen: %ld\n",
// 			strerror(errno), plen);
// 		exit(1);
// 	}
// 
// 	const unsigned len = align(m * n * sizeof(eltype), plen);
// 
// 	const int shmid = shmget(IPC_PRIVATE, len, SHM_HUGETLB | SHM_R | SHM_W);
// 	if(shmid > 0) {} else
// 	{
// 		eprintf("err: %s. can't get shm of len %u\n",
// 			strerror(errno), len);
// 		exit(1);
// 	}
// 
// 	void *const ptr = shmat(shmid, NULL, 0);
// 	if((intptr_t)ptr != -1) {} else
// 	{
// 		eprintf("err: %s. can't attach shm\n", strerror(errno));
// 	}
// 
// 	return ptr;
// }

int main(int argc, const char *const *const argv)
{
	setup.cfg = fillconfig(argc, argv);
	const unsigned sz = setup.cfg.size;
	const unsigned nw = setup.cfg.nworkers;

	printf("nworkers: %u; matrix size: %fMiB\n",
		nw, (double)sz * sz * sizeof(eltype) / (double)(1 << 20));

	setup.a = matalloc(sz, sz);
	setup.b = matalloc(sz, sz);
	setup.r = matalloc(sz, sz);

	printf("allocated\n");

	printf("randomization\n"); fflush(stdout); fflush(stderr);
	runjobs(nw, randroutine);

	printf("multiplication\n"); fflush(stdout); fflush(stderr);
	runjobs(nw, multroutine);

	printf("DONE\n");

	return 0;
}