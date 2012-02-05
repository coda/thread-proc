#include <matmul/mul.h>
#include <util/echotwo.h>

#include <stdio.h>

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

	for(unsigned i = 0; i < l; i += 1)
	for(unsigned j = 0; j < n; j += 1)
	{
		double sum = 0;

		for(unsigned k = 0; k < m; k += 1)
		{
			sum += a[i][k] * b[k][j];
		}

		r[i][j] = sum;
	}
}

void matfill(
	unsigned seed,
	const unsigned abr,
	eltype *const araw,
	const unsigned baserow,
	const unsigned l,
	const unsigned m,
	const unsigned tc,
	eltype (*const fn)(const unsigned, const unsigned, unsigned *const))
{
	eltype (*const a)[m] = (eltype (*const)[m])araw + baserow;

	for(unsigned i = 0; i < l; i += 1)
	for(unsigned j = 0; j < m; j += 1)
	{
		a[i][j] = fn(i + abr, j, &seed);
	}
}

static eltype matat(
	const eltype araw[],
	const unsigned n,
	const unsigned i,
	const unsigned j,
	const unsigned tr,
	const unsigned tc)
{
	return ((const eltype (*const)[n])araw)[i][j];
}

void matdump(
	const eltype araw[],
	const unsigned m,
	const unsigned n,
	const unsigned tr,
	const unsigned tc,
	const unsigned rbase,
	const unsigned cbase,
	const unsigned rlen,
	const unsigned clen)
{
	if(rbase + rlen < m && cbase + clen < n) { } else
	{
		fail("wrong rbl: (%u;%u) or cbl (%u;%u) for (%u;%u) matrix",
			rbase, rlen, cbase, clen, m, n);
	}

	if(n % tc == 0) { } else
	{
		fail("will not work with n: %u not aligned on tc: %u", n, tc);
	}

	for(unsigned i = rbase; i < rbase + rlen; i += 1)
	{
//		printf("\t");
		for(unsigned j = cbase; j < cbase + clen; j += 1)
		{
			printf("%f ", (double)matat(araw, n, i, j, tr, tc));
		}
		printf("\n");
	}
}
