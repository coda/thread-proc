#ifndef mulhincluded
#define mulhincluded

typedef double eltype; // matrix element type

enum
{
	tilelen = 4096,
	tilecount = tilelen / sizeof(eltype),
	tilerows = 32,
	tilecols = tilecount / tilerows
};

extern void matmul(
	const eltype amtx[],
	const eltype bmtx[],
	const unsigned baserow, // offset in rows from amtx and rmtx
	const unsigned l, // amount of work (nrows of a) is accounted here
	const unsigned m,
	const unsigned n,
	eltype rmtx[]);

extern void matrand(
	unsigned id,
	const unsigned absolutebaserow,
	eltype mtx[],
	const unsigned baserow,
	const unsigned m,
	const unsigned n,
	const unsigned tilecolumns);

// extern eltype matat(
// 	const eltype mtx[],
// 	const unsigned m,
// 	const unsigned i,
// 	const unsigned j,
// 	const unsigned tr,
// 	const unsigned tc);

extern void matdump(
	const eltype araw[],
	const unsigned m,
	const unsigned n,
	const unsigned tr,
	const unsigned tc,
	const unsigned rbase,
	const unsigned cbase,
	const unsigned rlen,
	const unsigned blen);


#endif
