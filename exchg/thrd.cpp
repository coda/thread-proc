extern "C" {
#include <./util.h>
#include <./work.h>

#include <stdio.h>
#include <stdlib.h>

// #include <poll.h>
}

#include <vector>
#include <iostream>
#include <algorithm>

#include <pthread.h>

using namespace std;

struct elvector
{
	vector<eltype> v;
	unsigned char padding[defpad(sizeof(v), cachelinelength)];
};

struct jobspec
{
	elvector * arrays;
	const testconfig * cfg;
	unsigned id;
	ringlink rl;

	jobspec(
		const unsigned i,
		const testconfig *const c,
		elvector *const a,
		const ringlink l) : arrays(a), cfg(c), id(i)
	{
//		rl.listening = 1;
		rl.writable = 1;
		rl.nexchanges = 0;
		rl.toread = l.toread;
		rl.towrite = l.towrite;
	};

	~jobspec() // it isn't virtual, will not be inherited
	{
		droprlink(&rl);
	}
};

static unsigned expand(
	vector<eltype>& array, ringlink *const,
	const unsigned id, const unsigned n);

static unsigned shrink(
	vector<eltype>&, ringlink *const,
	const unsigned, const unsigned);

static unsigned exchange(
	vector<eltype>&, ringlink *const,
	const unsigned, const unsigned);

static unsigned (*const functions[])(
	vector<eltype>&, ringlink *const, const unsigned,
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

static void * routine(void * arg)
{
	jobspec *const j = (jobspec *const)arg;

// 	printf("unit %03u is here with. rd: %d; wr: %d; nit: %u; nwrk: %u\n",
// 		j->id,
// 		j->rl.toread, j->rl.towrite,
// 		j->cfg->niterations, j->cfg->nworkers);
 
	const unsigned iters = j->cfg->niterations / j->cfg->nworkers;

//	cout << "routine " << j->id << " with " << iters << "iterations\n";

	unsigned seed = j->id;

//	tr1::uniform_int<unsigned> unif(0, 3);

	unsigned id = j->id;
	unsigned previd = id;

	unsigned i = 0;

	try {
		for(i = 0; id != (unsigned)-1 && i < iters; i += 1)
		{
	//		cout << unif(eng);

			const unsigned r = rand_r(&seed);
			const unsigned fn = r % nfunctions;

			previd = id;
			id = functions[fn](j->arrays[previd].v, &j->rl, id, r);
		}
	}
	catch(const exception e)
	{
		fail("unit %03u exception: %s\n", e.what());
	}

	uiwrite(j->rl.towrite, (unsigned)-1);

 	printf("unit %03u done. iters: %u; exchanges: %u\n",
 		j->id, i, j->rl.nexchanges);

	delete j;

	pthread_exit(NULL);
}

static void runjobs(
	const testconfig *const cfg,
	elvector *const arrays,
	void * (*const code)(void *))
{
	const unsigned count = cfg->nworkers;
	pthread_t *const threads = new pthread_t[count];

	unsigned ok = 1;
	unsigned err = 0;

	int p0rd = -1;
	int p0wr = -1;
	int p1rd = -1;
	int p1wr = -1;

	int zerord = -1;
	int zerowr = -1;

	unsigned i;

	// explicit external ifs would be too complex (if(count > 0) and so on).
	// Compiler should infer them during optimization
	for(i = 0; ok && i < count; i += 1)
	{
		if(i != 0)
		{
			p0rd = p1rd;
			p0wr = p1wr;
		}
		else
		{
			makerlink(&p0wr, &p0rd);
			zerord = p0rd;
			zerowr = p0wr;
		}

		if(i != count - 1)
		{
			makerlink(&p1wr, &p1rd);
		}
		else
		{
			p1wr = zerowr;
			p1rd = zerord;
		}

		ringlink rl;
		rl.toread = p0rd;
		rl.towrite = p1wr;

		err = pthread_create
		(
			threads + i,
			NULL,
			code,
			(void *)(new jobspec(i, cfg, arrays, rl))
		);

		ok = err == 0;
	}

	if(ok) {} else
	{
		fail("can't start %u jobs", count);
	}

	printf("%u jobs stated\n", i);

	for(i = 0; ok && i < count; i += 1)
	{
		err = pthread_join(threads[i], NULL);
		ok = err == 0;
		
//		eprintf("thread %u joined\n", i); // fflush(stderr);
	}

	if(ok) {} else
	{
		fail("can't join %u jobs", count);
	}

	printf("%u jobs joined\n", i); // fflush(stdout);

	delete[] threads;
}

static void process(const testconfig *const cfg)
{
	elvector *const arrays = new elvector[cfg->nworkers];

	runjobs(cfg, arrays, routine);

	printf("some values\n");
	for(unsigned i = 0; i < cfg->nworkers; i += 1)
	{
		if(arrays[i].v.size())
		{
			printf("\t%f\n", arrays[i].v[0]);
		}
		else
		{
			printf("\tEMPTY\n");
		}
	}

	delete[] arrays;
}

int main(const int argc, const char *const *const argv)
{
	ignoresigpipe();

	const testconfig cfg = fillconfig(argc, argv);

	try
	{
		process(&cfg);
	}
	catch(const exception& e)
	{
		fail("something wrong. %s\n", e.what());
	}

	printf("DONE. vector<eltype> size: %lu; elvector size %lu\n",
		sizeof(vector<eltype>), sizeof(elvector));	

	return 0;
}

static unsigned expand(
	vector<eltype>& array, ringlink *const rl,
	const unsigned id, const unsigned r)
{
	const unsigned n = r % workfactor;
	unsigned seed = r;

	array.reserve(array.size() + n);
	for(unsigned i = 0; i < n; i += 1)
	{
		array.push_back(elrand(&seed));
	}

	return id;
}

static unsigned shrink(
	vector<eltype>& array, ringlink *const rl,
	const unsigned id, const unsigned r)
{
	const unsigned n = min((unsigned long)(r % workfactor), array.size());

	if(n > 0)
	{
// 		eprintf("will summ %u of %u els. starting: %p\n",
// 			n, array.size(), &array[0]);

		array.push_back(heapsum(&array[0], n));

		vector<eltype>::iterator b = array.begin();
		array.erase(b, b + n);
	}

	return id;
}

// static unsigned exchange(
// 	vector<eltype>& array, ringlink *const rl,
// 	const unsigned id, const unsigned n)
// {
// 
// 	eprintf("exchange here\n");
// 
// 	if(rl->listening) {} else // not listening
// 	{
// 		return id;
// 	}
// 
// // 	struct pollfd pfd;
// // 	pfd.fd = rl->toread;
// // 	pfd.events = POLLIN;
// // 
// // 	const int rv = poll(fds, 2, 0);
// // 	if(rv > 0)
// // 	{
// // 	}
// // 	else if(rv == 0)
// // 	{
// // 		return id; // no data
// // 	}
// 
// 	// uiread will fail on the EOF, but end of communication is handled
// 	// differently; so
// 	const unsigned i = uiread(rl->toread);
// 	
// 	if(i != (unsigned)-1) {} else // end of comm
// 	{
// 		uiwrite(rl->towrite, i);
// 		rl->listening = 0;
// 		printf("got end of comm\n");
// 
// 		return id;
// 	}
// 
// 	uiwrite(rl->towrite, id);
// 
// 	return i;
// }

static unsigned exchange(
	vector<eltype>& array, ringlink *const rl,
	const unsigned id, const unsigned n)
{
	rl->nexchanges += 1;

//	eprintf("exchange %d -> %d\n", rl->toread, rl->towrite);
	if(rl->writable)
	{
		rl->writable = uiwrite(rl->towrite, id);
	}

	return uiread(rl->toread);
}
