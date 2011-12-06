#include <./mul.h>
#include <./util.h>

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <pthread.h>
#include <errno.h>

static struct
{
	eltype * a;
	eltype * b;
	eltype * r;

	testconfig cfg;
} setup;

// typedef struct
// {
// 	unsigned startrow;
// 	unsigned nrows;
// } jobitem;

// static jobitem ballance(
// 	const unsigned id,
// 	const unsigned nwrks,
// 	const unsigned nrows)
// {
// 	unsigned sr = 0;
// 	unsigned l = 0;
// 
// 	if(nrows > nwrks)
// 	{
// 		const unsigned chunk = nrows / nwrks;
// 
// 		sr = chunk * id;
// 		if(id < nwrks - 1) // not the last worker
// 		{
// 			l = chunk;
// 		}
// 		else // the last one takes the rest
// 		{
// 			l = nrows - sr;
// 		}
// 	}
// 	else // one row per worker
// 	{
// 		if(id < nrows)
// 		{
// 			sr = id;
// 			l = 1;
// 		}
// 	}
// 
// 	return (jobitem){ .startrow = sr, .nrows = l };
// }

static void * multroutine(void * arg)
{
	const unsigned id = (uintptr_t)arg;
	const unsigned sz = setup.cfg.size;
	const unsigned nwrks = setup.cfg.nworkers;

	const unsigned m = sz;
	const unsigned n = sz;

	const jobitem ji = ballance(id, nwrks, sz);
	const unsigned l = ji.nrows;
	const unsigned startrow = ji.startrow;

// 	const eltype *const a = setup.a + startrow * sizeof(eltype) * m;
// 	eltype *const r = setup.r + startrow * sizeof(eltype) * n;
//	MEGAFAIL: they are already have basetype of eltype

	const eltype *const a = setup.a + startrow * m;
	eltype *const r = setup.r + startrow * n;

	matmul(a, setup.b, l, m, n, r);

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
	const unsigned startrow = ji.startrow;

	eltype *const a = setup.a + startrow * m;
	eltype *const b = setup.b + startrow * n;

	matrand(id, a, l, m);
	matrand(id * 5, b, l, n); // because matricies are square.

	printf("rand %u with %u rows is done\n", id, l);

	return NULL;
}

static eltype * matalloc(unsigned m, unsigned n)
{
	return malloc(m * n * sizeof(eltype));
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

	free(a);
	free(b);
	free(r);

	printf("main DONE\n");

	return 0;
}
