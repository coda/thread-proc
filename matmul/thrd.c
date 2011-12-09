#include <./mul.h>
#include <./util.h>

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <pthread.h>
#include <errno.h>

#include <unistd.h>
#include <sys/shm.h>

static struct
{
	eltype * a;
	eltype * b;
	eltype * r;

	testconfig cfg;
} setup;

static void * multroutine(void * arg)
{
	const unsigned id = (uintptr_t)arg;
	const unsigned sz = setup.cfg.size;
	const unsigned nwrks = setup.cfg.nworkers;

	const unsigned m = sz;
	const unsigned n = sz;

	const jobitem ji = ballance(id, nwrks, sz);
	const unsigned l = ji.nrows;
//	const unsigned sr = ji.startrow;

	// the names should be transposed	
	const unsigned sr = aligndown(ji.startrow, tilerows);
	const unsigned baserow = ji.startrow - sr;

	const eltype *const a = setup.a + sr * m;
	eltype *const r = setup.r + sr * n;

	matmul(a, setup.b, baserow, l, m, n, r);

	printf("mult %u with %u rows is done\n", id, l);

	return NULL;
}

static void * randroutine(void * arg)
{
	const unsigned id = (uintptr_t)arg;
	const unsigned sz = setup.cfg.size;
	const unsigned nwrks = setup.cfg.nworkers;

	const unsigned m = sz;
	const unsigned n = sz;

	const jobitem ji = ballance(id, nwrks, sz);
	const unsigned l = ji.nrows;
//	const unsigned startrow = ji.startrow;

	const unsigned sr = aligndown(ji.startrow, tilerows);
	const unsigned baserow = ji.startrow - sr;

	eltype *const a = setup.a + sr * m;
	eltype *const b = setup.b + sr * n;

	// because matricies are square, but not tiles
	matrand(id, a, baserow, l, m, tilecols);
	matrand(id * 5, b, baserow, l, n, tilerows); 

	printf("rand %u with %u rows is done\n", id, l);

	return NULL;
}

static eltype * matalloc(unsigned m, unsigned n)
{
 	return malloc(m * n * sizeof(eltype));
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
// 	const int shmid = shmget(IPC_PRIVATE, len, SHM_R | SHM_W);
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

static void runjobs(const unsigned count, void * (* routine)(void *))
{
	pthread_t threads[count];
	unsigned ok = 1;
	unsigned err = 0;

	for(unsigned i = 0; ok && i < count; i += 1)
	{
		err = pthread_create(threads + i, NULL,
			routine, (void *)(uintptr_t)i);
		ok = err == 0;
	}

	if(ok) {} else
	{
		eprintf("err: %s. can't start %u jobs\n", strerror(err), count);
		exit(1);
	}

	for(unsigned i = 0; ok && i < count; i += 1)
	{
		err = pthread_join(threads[i], NULL);
		ok = err == 0;
	}

	if(ok) {} else
	{
		eprintf("err: %s. can't join %u jobs\n", strerror(err), count);
		exit(1);
	}
}

int main(int argc, const char *const *const argv)
{
	setup.cfg = fillconfig(argc, argv);	
	const unsigned sz = setup.cfg.size;
	const unsigned nw = setup.cfg.nworkers;

	printf("nworkers: %u; matrix size: %fMiB\n",
		nw, (double)sz * sz * sizeof(eltype) / (double)(1 << 20));

	void *const a = setup.a = matalloc(sz, sz);
	void *const b = setup.b = matalloc(sz, sz);
	void *const r = setup.r = matalloc(sz, sz);

	if(a != NULL && b != NULL && r != NULL) {} else
	{
		eprintf("err: %s. can't allocate matricies\n", strerror(errno));
		exit(1);
	}

	printf("allocated. a: %p; b: %p; r: %p\n", a, b, r);

	printf("randomization\n");
	runjobs(nw, randroutine);

	printf("multiplication\n");
	runjobs(nw, multroutine);

// 	free(a);
// 	free(b);
// 	free(r);

	printf("main DONE\n");

	return 0;
}
