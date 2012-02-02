#include <matmul/mul.h>
#include <matmul/muljob.h>
#include <util/config.h>
#include <util/spawn.h>
#include <util/echotwo.h>

#include <stdlib.h>
#include <stdio.h>

#include <sched.h>

typedef struct
{
	eltype * a;
	eltype * b;
	eltype * r;
} workset;

static void multroutine(const void *const arg)
{
	const idargument *const ia = (idargument *)arg;
	const workset *const ws = ia->tp->extra;
	const runconfig *const rc = ia->tp->rc;
	
	const unsigned id = ia->id;
	const unsigned sz = rc->size;

	const unsigned l = sz;
	const unsigned m = sz;
	const unsigned n = sz;

	const unsigned tr = tilerows;
	const unsigned tc = tilecols;

	const joblayout al = definejob(rc, id, l, m, tr, tc);
	const joblayout rl = definejob(rc, id, l, n, tr, tr);

	const eltype *const a = ws->a + al.baseoffset / sizeof(eltype);
	eltype *const r = ws->r + rl.baseoffset / sizeof(eltype);

	matmul(a, ws->b, al.baserow, al.nrows, m, n, r);

	printf("mult %u with %u rows is done on core %d\n", id, al.nrows,
		sched_getcpu());
}

static void randroutine(const void *const arg)
{
	const idargument *const ia = (idargument *)arg;
	const workset *const ws = ia->tp->extra;
	const runconfig *const rc = ia->tp->rc;
	
	const unsigned id = ia->id;
	const unsigned sz = rc->size;

	const unsigned l = sz;
	const unsigned m = sz;
	const unsigned n = sz;

	const unsigned tr = tilerows;
	const unsigned tc = tilecols;

	const joblayout al = definejob(rc, id, l, m, tr, tc);
	const joblayout bl = definejob(rc, id, m, n, tc, tr);

	eltype *const a = ws->a + al.baseoffset / sizeof(eltype);
	eltype *const b = ws->b + bl.baseoffset / sizeof(eltype);

	matrand(id, al.absolutebaserow, a, al.baserow, al.nrows, m, tc);
	matrand(id * 5, al.absolutebaserow, b, bl.baserow, bl.nrows, n, tr); 

	printf("rand %u with %u rows is done on core %d\n", id, al.nrows,
		sched_getcpu());
}

static eltype * matalloc(unsigned m, unsigned n)
{
 	void *const ptr = malloc(m * n * sizeof(eltype));

	if(ptr != NULL) { } else
	{
		fail("can't allocate matrix of m: %u n: %u", m, n);
	}

	return ptr;
}

int main(const int argc, const char *const argv[])
{
	const runconfig *const rc = formconfig(argc, argv, 64, 1024);
	const unsigned sz = rc->size;
	if(sz >= 512) { } else
	{
		fail("problem size %u is too small", sz);
	}

	printf("\tmatrix size: %fMiB\n",
		(double)sz * sz * sizeof(eltype) / (double)(1 << 20));

	const workset ws = {
		.a = matalloc(sz, sz),
		.b = matalloc(sz, sz),
		.r = matalloc(sz, sz) };

	void *const a = ws.a;
	void *const b = ws.b;
	void *const r = ws.r;

	printf("\tallocated. a: %p; b: %p; r: %p\n", a, b, r);

	treeplugin tp = {
		.makeargument = makeidargument,
		.dropargument = dropidargument,
		.rc = rc,
		.extra = (void *)&ws };

	printf("randomization\n");
	tp.treeroutine = randroutine;
	treespawn(&tp);

	printf("multiplication\n");
	tp.treeroutine = multroutine;
	treespawn(&tp);

	printf("some values\n");

	const unsigned tr = tilerows;
	const unsigned tc = tilecols;

	matdump(a, sz, sz, tr, tc, 0, 0, 9, 9);
	
	free(r);
	free(b);
	free(a);

	return 0;
}
