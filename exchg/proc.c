#include <./util.h>
#include <./work.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>

typedef struct
{
	int fd;
	unsigned offset; // in bytes
	unsigned length; // of data, the file length will be detected with lseek
} vectorfile;

static eltype vfelat(const vectorfile *const vf, const unsigned i)
{
	const unsigned elcnt = vf->length / sizeof(eltype);

	if(i < elcnt) {} else
	{
		fail("idx: %i is out of bounds", i);
	}

//	eprintf("vf->fd length: %u\n", flength(vf->fd));

	off_t pos = vf->offset + i * sizeof(eltype);
	if(lseek(vf->fd, pos, SEEK_SET) == pos) {} else
	{
		fail("can't seek to pos:%u = off:%u + i:%u",
			pos, vf->offset, i * sizeof(eltype));
	}

	eltype value;
	if(read(vf->fd, &value, sizeof(value)) == sizeof(value)) {} else
	{
		fail("can't read");
	}

	return value;
}

typedef struct
{
	vectorfile vf;
	unsigned char padding[defpad(sizeof(vectorfile), cachelinelength)];
} elvector;

typedef struct
{
	char * ptr;
	unsigned capacity; // overall in bytes
	unsigned offset;
	unsigned length;
} vector;

// returns pointer to the beginning of space added
static eltype * vectorexpand(vector *const v, const unsigned n,
	const unsigned plen)
{
//	eprintf("expand rq for %u\n", n);

	const unsigned need = v->offset + v->length + n * sizeof(eltype);

	if(need < v->capacity)
	{
		return (eltype *)(v->ptr + v->offset + v->length);
	}

//	eprintf("will really expand\n");

	void *const ptr
		= mremap(v->ptr, v->capacity,
			align(need, plen), MREMAP_MAYMOVE);
	
	if(ptr != MAP_FAILED) {} else
	{
		fail("can't perform expanding remap");
	}

	v->capacity = align(need, plen);
	v->ptr = ptr;

//	eprintf("expanded capacity: %u\n", v->capacity);

	return (eltype *)(v->ptr + v->offset + v->length);
}

static void vectorupload(const vector *const v, vectorfile *const vf)
{
	vf->offset = v->offset;
	vf->length = v->length;

	if(pwrite(vf->fd, v->ptr, v->capacity, 0) == v->capacity) {} else
	{
		fail("uploading. can't write old data");
	}
}

static void vectordownload(const vectorfile *const vf, vector *const v)
{
	const unsigned len = flength(vf->fd);

	if(len <= v->capacity) {} else
	{
		void *const ptr
			= mremap(v->ptr, v->capacity, len, MREMAP_MAYMOVE);

		if(ptr != MAP_FAILED) {} else
		{
			fail("reloading. can't remap vector");
		}

		v->ptr = ptr;
		v->capacity = len;
	}

	v->offset = vf->offset;
	v->length = vf->length;

	if(pread(vf->fd, v->ptr, len, 0) == len) {} else
	{
		fail("downloading. can't download vector");
	}
}

#define actionfunction(fname) unsigned fname \
( \
	ringlink *const rl, \
	vectorfile *const vf, vector *const v, \
	const unsigned id, \
	const unsigned r \
)

static actionfunction(expand);
static actionfunction(shrink);
static actionfunction(exchange);

static actionfunction((*const functions[])) =
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
	const unsigned jid)
{
	const testconfig *const cfg = rl->cfg;
	const unsigned iters = cfg->niterations / cfg->nworkers;

//	const int fd = vectors[jid].vf.fd;
//	const unsigned len = flength(fd);	
	const unsigned len = cfg->pagelength;
	vector v = {
		.ptr = peekmap(cfg, -1, 0, len, pmwrite | pmprivate),
		.capacity = len,
		.length = 0,
		.offset = 0 };

	unsigned seed = jid;
	unsigned id = jid;
	unsigned i;
	for(i = 0; id != (unsigned)-1 && i < iters; i += 1)
	{
			const unsigned r = rand_r(&seed);
			const unsigned fn = r % nfunctions;

			id = functions[fn](rl, &vectors[id].vf, &v, id, r);
	}

	uiwrite(rl->towrite, (unsigned)-1);

 	printf("unit %03u(%u) done. iters: %u; exchanges: %u\n",
 		jid, getpid(), i, rl->nexchanges);
}

static void runjobs
(
	const testconfig *const cfg,
	elvector *const vectors,
	void (*const code)(ringlink *const, elvector *const, unsigned)
) {
	const unsigned count = cfg->nworkers;
	pid_t procs[count];

	unsigned ok = 1;
	unsigned err = 0;

	int p0rd = -1;
	int p1rd = -1;
	int p1wr = -1;

	int zerowr = -1;

	unsigned i;

	// explicit external ifs would be too complex (if(count > 0) and so on).
	// Compiler should infer them during optimization
	for(i = 0; ok && i < count; i += 1)
	{
		if(i != 0)
		{
			p0rd = p1rd;
		}
		else
		{
			makerlink(&zerowr, &p0rd);
		}

		if(i != count - 1)
		{
			makerlink(&p1wr, &p1rd);
		}
		else
		{
			p1wr = zerowr;
		}

		pid_t p = fork();
		if(p == 0)
		{
			ringlink rl = {
				.toread = p0rd,
				.towrite = p1wr,
				.nexchanges = 0,
				.writable = 1,
				.cfg = cfg };

			if(i != count - 1)
			{
				uclose(zerowr);
				uclose(p1rd);
			}

			code(&rl, vectors, i);
			exit(0);
		}
		else if(p > 0)
		{
			procs[i] = p;

			uclose(p0rd);
			uclose(p1wr);
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
		int status;
		const pid_t p = waitpid(procs[i], &status, 0);
		ok = p == procs[i];

		eprintf("joined %d with ifsig: %u; termsig: %s\n",
			p, WIFSIGNALED(status), strsignal(WTERMSIG(status)));
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
		= (elvector *)peekmap(&cfg, -1, 0, vectslen, pmwrite);

	for(unsigned i = 0; i < cfg.nworkers; i += 1)
	{
//		// makeshm will align 1 up to pagelength
//		vectors[i].vf.fd = makeshm(&cfg, 1);

		vectors[i].vf.fd = makeshm(&cfg, 0);
		vectors[i].vf.length = 0;
		vectors[i].vf.offset = 0;
	}

	fflush(stderr);
	fflush(stdout);
	runjobs(&cfg, vectors, routine);

	printf("some values\n");

	for(unsigned i = 0; i < cfg.nworkers; i += 1)
	{
		const unsigned len = vectors[i].vf.length / sizeof(eltype);
		if(len)
		{
			printf("\t%f\t%u of %u\n",
				(double)vfelat(&vectors[i].vf, len / 2),
				len / 2,
				len);
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

actionfunction(expand) // rl, vf, v, id, r
{
	const unsigned n = r % workfactor;
	unsigned seed = r;
	
	eprintf("before expand. cap: %u\n", v->capacity);
	eltype *const buf = vectorexpand(v, n, rl->cfg->pagelength);
	eprintf("after expand. cap: %u\n", v->capacity);

	for(unsigned i = 0; i < n; i += 1)
	{
		buf[i] = elrand(&seed);
	}

	v->length += n * sizeof(eltype);

	return id;
}

actionfunction(shrink)
{
	return id;
}

actionfunction(exchange)
{
// 	rl->nexchanges += 1;
// 	
//  	vf->length = v->length;
//  	vf->offset = v->offset;
//  
//  	if(pwrite(vf->fd, v->ptr, v->capacity, 0) == v->capacity) {} else
//  	{
//  		fail("can't write for exchange. fd: %d; capacity: %u",
//  			vf->fd, v->capacity);
//  	}
// 
// 	if(munmap(v->ptr, v->capacity) == 0) {} else
// 	{
// 		fail("can't unmap used vector: ptr: %x; capacity: %u",
// 			v->ptr, v->capacity);
// 	}
// 
// 	if(rl->writable)
// 	{
// 		rl->writable = uiwrite(rl->towrite, id);
// 	}
// 
// 	const unsigned i = uiread(rl->toread);
// 
// 	if(i != (unsigned)-1)
// 	{
// 		// FIXME: quite a dirty hack
// 		const vectorfile *const ivf = &((elvector *)vf - id + i)->vf; 
// 		
// 		v->offset = ivf->offset;
// 		v->length = ivf->length;
// 		v->capacity = flength(ivf->fd);
// 
// 		eprintf("vf: %u; capacity: %u\n", i, v->capacity);
// 
// 		v->ptr = peekmap(rl->cfg,
// 			ivf->fd, 0, v->capacity, pmwrite | pmprivate);
// 	}
// 

	rl->nexchanges += 1;

	vectorupload(v, vf);

	if(rl->writable)
	{
		rl->writable = uiwrite(rl->towrite, id);	
	}
	
	const unsigned i = uiread(rl->toread);

	if(i != (unsigned)-1)
	{
		const vectorfile *const ivf = &(((elvector *)vf) - id + i)->vf;
		vectordownload(ivf, v);
	}

	return i;
}
