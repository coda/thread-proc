#include <./mul.h>
#include <./util.h>

#include <stdio.h>
#include <stdlib.h>

void matmul(
	const eltype *const araw,
	const eltype *const braw,
	const unsigned baserow,
	const unsigned l, // amount of work (nrows of a) is accounted here
	const unsigned m,
	const unsigned n,
	eltype *const rraw)
{
	const eltype (*const a)[m] = (const eltype (*const)[m])araw + baserow;
	const eltype (*const b)[n] = (const eltype (*const)[n])braw;
	eltype (*const r)[n] = (eltype (*const)[n])rraw + baserow;

// 	eprintf("matmul. l: %u; m: %u; n: %u; a: %p; b: %p; r: %p\n",
// 		l, m, n, a, b, r); fflush(stderr);

	for(unsigned i = 0; i < l; i += 1)
	for(unsigned j = 0; j < n; j += 1)
	{
//		r[i][j] = 0;
		double sum = 0;

		for(unsigned k = 0; k < m; k += 1)
		{
//			r[i][j] += a[i][k] * b[k][j];
			sum += a[i][k] * b[k][j];
		}

//		volatile double *volatile ptr = &r[i][j];
//		*ptr = sum;
//		*ptr = sum;

		r[i][j] = sum;
	}
}

void matrand(
	unsigned seed,
	eltype *const araw,
	const unsigned baserow,
	const unsigned l,
	const unsigned m,
	const unsigned tc)
{
	eltype (*const a)[m] = (eltype (*const)[m])araw + baserow;

	for(unsigned i = 0; i < l; i += 1)
	for(unsigned j = 0; j < m; j += 1)
	{
		a[i][j] = 1.0 / rand_r(&seed);
	}
}


