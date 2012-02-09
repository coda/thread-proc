#include <matmul/mul.h>
#include <matmul/muljob.h>
#include <util/config.h>
#include <util/spawn.h>
#include <util/echotwo.h>
#include <util/memfile.h>
#include <util/memmap.h>

#include <stdio.h>
#include <stdlib.h>

#include <sched.h>

typedef struct
{
 	int fda;
 	int fdb;
 	int fdr;
} workset;

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

	const unsigned adiff = (al.baseoffset - al.mapoffset) / sizeof(eltype);
	eltype *const a
		= (eltype *)peekmap(rc, ws->fda, al.mapoffset, al.maplength,
			pmwrite | pmshared)
		+ adiff;

	const unsigned bdiff = (bl.baseoffset - bl.mapoffset) / sizeof(eltype);
	eltype *const b
		= (eltype *)peekmap(rc, ws->fdb, bl.mapoffset, bl.maplength,
			pmwrite | pmshared)
		+ bdiff;

	matfill(id, al.absolutebaserow, a, al.baserow, al.nrows, m, tc,
		elrand);

	matfill(id * 5, bl.absolutebaserow, b, bl.baserow, bl.nrows, n, tr,
		elrand);

	printf("rand %03u with a:%u b:%u rows is done on core %d\n",
		id, al.nrows, bl.nrows, sched_getcpu());
}

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
	const unsigned adiff = (al.baseoffset - al.mapoffset) / sizeof(eltype);

	runconfig brc = *rc;
	brc.nworkers = 1;

	const joblayout bl = definejob(&brc, 0, m, n, tc, tr);

	const joblayout rl = definejob(rc, id, l, n, tr, tr);
	const unsigned rdiff = (rl.baseoffset - rl.mapoffset) / sizeof(eltype);

	const eltype *const a
		= (const eltype *)peekmap(rc, ws->fda,
			al.mapoffset, al.maplength, pmshared)
		+ adiff;

	const eltype *const b
		= (const eltype *)peekmap(rc, ws->fdb,
			bl.mapoffset, bl.maplength, pmshared);

	eltype *const r
		= (eltype *)peekmap(rc, ws->fdr,
			rl.mapoffset, rl.maplength, pmwrite | pmshared)
		+ rdiff;

	matmul(a, b, al.baserow, al.nrows, m, n, r);

	printf("mult %03u with a:%u rows is done on core %d\n",
		id, al.nrows, sched_getcpu());
}

int main(const int argc, const char *const argv[])
{
	const runconfig *const rc = formconfig(argc, argv, 64, 1024);
	const unsigned sz = rc->size;

	printf("\tmatrix size: %fMiB\n",
		(double)sz * sz * sizeof(eltype) / (double)(1 << 20));

	const workset ws = {
		.fda = makeshm(rc, sz * sz * sizeof(eltype)),
		.fdb = makeshm(rc, sz * sz * sizeof(eltype)),
		.fdr = makeshm(rc, sz * sz * sizeof(eltype)) };

	treeplugin tp = {
		.makeargument = makeidargument,
		.dropargument = dropidargument,
		.rc = rc,
		.extra = (void *)&ws };

	printf("randomization\n");
	fflush(stdout);

	tp.treeroutine = randroutine;
	treespawn(&tp);

	printf("multiplication\n");
	fflush(stdout);

	tp.treeroutine = multroutine;
	treespawn(&tp);

	dropshm(ws.fda);
	dropshm(ws.fdb);

	printf("some values\n");

	const unsigned tr = tilerows;
	const unsigned m = sz;

	const eltype *const r
		= (const eltype *)peekmap(rc, ws.fdr,
			0, m * m * sizeof(eltype), 0);

	matdump(r, m, m, tr, tr, 127, 237, 8, 8);
	
	dropshm(ws.fdr);

	return 0;
}
