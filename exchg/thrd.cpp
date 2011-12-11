extern "C" {
#include <./util.h>
#include <./exchg.h>

#include <stdio.h>
#include <stdlib.h>
}

#include <vector>
#include <iostream>

#include <pthread.h>

using namespace std;

struct elvector
{
	vector<eltype> v;
	unsigned char padding[defpad(sizeof(v), cachelinelength)];
};

struct jobspec
{
	vector<eltype> * arrays;
	const testconfig * cfg;
	unsigned id;

	jobspec(
		const unsigned i,
		const testconfig *const c,
		vector<eltype> *const a) : arrays(a), cfg(c), id(i) {};
};

static void runjobs(
	const testconfig *const cfg,
	vector<eltype> *const arrays,
	void * (*const routine)(void *))
{
	const unsigned count = cfg->nworkers;
	pthread_t *const threads = new pthread_t[count];
	unsigned ok = 1;
	unsigned err = 0;

	for(unsigned i = 0; ok && i < count; i += 1)
	{
		err = pthread_create
		(
			threads + i,
			NULL,
			routine,
			new jobspec(i, cfg, arrays)
		);

		ok = err == 0;
	}

	if(ok) {} else
	{
		fail("can't start %u jobs", count);
	}

	for(unsigned i = 0; ok && i < count; i += 1)
	{
		err = pthread_join(threads[i], NULL);
		ok = err == 0;
	}

	if(ok) {} else
	{
		fail("can't join %u jobs", count);
	}
}

static void expand(vector<eltype>& array, unsigned int n)
{
}

static void shrink(vector<eltype>& array, unsigned int n)
{
}

static void exchange(vector<eltype>& array, unsigned int n)
{
}

static void (*const functions[])(vector<eltype>&, unsigned int) =
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

static void * routine(void * arg)
{
	const jobspec *const j = (const jobspec *const)arg;
	const unsigned iters = j->cfg->niterations / j->cfg->nworkers;

//	cout << "routine " << j->id << " with " << iters << "iterations\n";
	printf("routine %03u with %u iterations\n", j->id, iters);

	unsigned seed = j->id;

//	tr1::uniform_int<unsigned> unif(0, 3);

	for(unsigned i = 0; i < iters; i += 1)
	{
//		cout << unif();
		
		rand_r(&seed);
	}

	return NULL;
}

static void process(const testconfig *const cfg)
{
	vector<eltype> *const arrays = new vector<eltype>[cfg->nworkers];

	runjobs(cfg, arrays, routine);

	delete[] arrays;
}

int main(const int argc, const char *const *const argv)
{
	const testconfig cfg = fillconfig(argc, argv);

	try
	{
		process(&cfg);
	}
	catch(const exception& e)
	{
		fail("something wrong. %s\n", e.what());
	}

	printf("done. vector<eltype> size: %u; elvector size %u\n",
		sizeof(vector<eltype>), sizeof(elvector));	

	return 0;
}
