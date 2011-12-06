#include <./mul.h>

void matmul(
	const eltype *const araw,
	const eltype *const braw,
	const unsigned l, // amount of work (nrows of a) is accounted here
	const unsigned m,
	const unsigned n,
	eltype *const rraw)
{
	const eltype (*const a)[m] = (const eltype (*const)[m])araw;
	const eltype (*const b)[n] = (const eltype (*const)[n])braw;
	eltype (*const r)[n] = (eltype (*const)[n])rraw;

	for(unsigned i = 0; i < l; i += 1)
	for(unsigned j = 0; j < n; j += 1)
	{
		r[i][j] = 0;

		for(unsigned k = 0; k < m; k += 1)
		{
			r[i][j] += a[i][k] * b[k][j];
		}
	}
}
