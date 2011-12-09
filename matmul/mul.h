#ifndef mulhincluded
#define mulhincluded

typedef double eltype; // matrix element type

enum
{
	tilesize = 4096,
	tilerows = 32,
	tilecols = tilesize / sizeof(eltype) / tilerows
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
	eltype *const mtx,
	const unsigned baserow,
	const unsigned m,
	const unsigned n);

#endif
