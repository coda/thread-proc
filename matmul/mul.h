#ifndef mulhincluded
#define mulhincluded

typedef double eltype; // matrix element type

extern void matmul(
	const eltype amtx[],
	const eltype bmtx[],
	const unsigned l, // amount of work (nrows of a) is accounted here
	const unsigned m,
	const unsigned n,
	eltype rmtx[]);

extern void matrand(eltype *const mtx, unsigned m, unsigned n);

#endif
