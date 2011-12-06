#include <./mul.h>
#include <./util.h>

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include <pthread.h>

// static struct
// {
// 	eltype * a;
// 	eltype * b;
// 	eltype * r;
// 	unsigned size;
// 	unsigned nworkers;
// } cfg;

static struct
{
	eltype * a;
	eltype * b;
	eltype * r;

	testconfig cfg;
} setup;

typedef struct
{
	unsigned startrow;
	unsigned nrows;
} jobitem;

static jobitem ballance(
	const unsigned id,
	const unsigned nwrks,
	const unsigned nrows)
{
	unsigned sr = 0;
	unsigned l = 0;

	if(nrows > nwrks)
	{
		const unsigned chunk = nrows / nwrks;

		sr = chunk * id;
		if(id < nwrks - 1) // not the last worker
		{
			l = chunk;
		}
		else // the last one takes the rest
		{
			l = nrows - sr;
		}
	}
	else // one row per worker
	{
		if(id < nrows)
		{
			sr = id;
			l = 1;
		}
	}

	return (jobitem){ .startrow = sr, .nrows = l };
}

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

	const eltype *const a = setup.a + startrow * sizeof(eltype) * m;
	eltype *const r = setup.r + startrow * sizeof(eltype) * n;
	matmul(a, setup.b, l, m, n, r);

	eprintf("mult %u with l rows %u is done\n", id, l);

	return NULL;
}

static eltype * matalloc(unsigned m, unsigned n)
{
	return malloc(m * n * sizeof(eltype));
}

int main(int argc, char ** argv)
{
	setup.cfg = fillconfig(argc, argv);	
	const unsigned sz = setup.cfg.size;
	const unsigned nw = setup.cfg.nworkers;

	eprintf("nworkers: %u; matrix size: %u\n", nw, sz);

	void *const a = setup.a = matalloc(sz, sz);
	void *const b = setup.b = matalloc(sz, sz);
	void *const r = setup.r = matalloc(sz, sz);

	if(a != NULL && b != NULL && c != NULL) {} else
	{
		eprintf("err: %s. can't allocate matricies\n", strerror(errno));
		exit(1);
	}

	pthread_t threads[nw];
	unsigned ok = 1;
	unsigned i;
	for(i = 0; ok && i < nw; i += 1)
	{
		ok = pthread_create(threads + i, NULL, routine, (void

	return 0;
}
