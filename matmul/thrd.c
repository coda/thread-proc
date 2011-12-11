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
//	const unsigned nwrks = setup.cfg.nworkers;

	const unsigned l = sz;
	const unsigned m = sz;
	const unsigned n = sz;

// 	const jobitem ji = ballance(id, nwrks, sz);
// 	const unsigned l = ji.nrows;
// //	const unsigned sr = ji.startrow;
// 
// 	// the names should be transposed	
// 	const unsigned sr = aligndown(ji.startrow, tilerows);
// 	const unsigned baserow = ji.startrow - sr;

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

	return NULL;
}

static void * randroutine(void * arg)
{
	const unsigned id = (uintptr_t)arg;
	const unsigned sz = setup.cfg.size;
//	const unsigned nwrks = setup.cfg.nworkers;

	const unsigned l = sz;
	const unsigned m = sz;
	const unsigned n = sz;

// 	const jobitem ji = ballance(id, nwrks, sz);
// 	const unsigned l = ji.nrows;
// //	const unsigned startrow = ji.startrow;
// 
// 	const unsigned sr = aligndown(ji.startrow, tilerows);
// 	const unsigned baserow = ji.startrow - sr;
// 
// 	eltype *const a = setup.a + sr * m;
// 	eltype *const b = setup.b + sr * n;
// 
// 	// because matricies are square, but not tiles
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

// 	printf("some values\n");
// 	for(unsigned i = 0; i < 8; i += 1)
// 	{
// 		printf("\t");
// 		for(unsigned j = 0; j < 8; j += 1)
// 		{
// 			printf("%f ", (double)matat(b, m, i, j, tilerows));
// 		}
// 		printf("\n");
// 	}

	return NULL;
}

static eltype * matalloc(unsigned m, unsigned n)
{
 	void *const ptr = malloc(m * n * sizeof(eltype));

	if(ptr != NULL) {} else
	{
		fail("can't allocate matrix of m: %u n: %u", m, n);
	}

	return ptr;
}

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

// 	if(a != NULL && b != NULL && r != NULL) {} else
// 	{
// 		eprintf("err: %s. can't allocate matricies\n", strerror(errno));
// 		exit(1);
// 	}

	printf("allocated. a: %p; b: %p; r: %p\n", a, b, r);

//	const unsigned tc = tilecols;
 	const unsigned tr = tilerows;
 	const unsigned m = sz;

	printf("randomization\n");
	runjobs(nw, randroutine);

	printf("multiplication\n");
	runjobs(nw, multroutine);

// 	free(a);
// 	free(b);
// 	free(r);

//	const unsigned tc = tilecols;
//	const unsigned m = sz;

	printf("some values\n");
	for(unsigned i = 237; i < 237 + 8; i += 1)
	{
		printf("\t");
		for(unsigned j = 15; j < 15 + 8; j += 1)
		{
			printf("%f ", (double)matat(r, m, i, j, tr, tr));
		}
		printf("\n");
	}

	return 0;
}
