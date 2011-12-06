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

extern void matrand(unsigned id,
	eltype *const mtx, const unsigned m, const unsigned n);

#endif
