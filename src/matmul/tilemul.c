#include <matmul/mul.h>
#include <matmul/util.h>

#include <stdlib.h>
#include <stdio.h>

void matmul(
	const eltype araw[],
	const eltype braw[],
	const unsigned baserow,
	const unsigned l,
	const unsigned m,
	const unsigned n,
	eltype rraw[])
{
	const unsigned tc = tilecols;
	const unsigned tr = tilerows;

	if(m % tc || n % tr)
	{
		fail("won't  multiply. m: %u; n: %u are not aligned "
			"on tc: %u; tr: %u\n",
			l, m, n, tc, tr);
	}

	const eltype (*const a)[m / tc][tr][tc]
		= (const eltype (*const)[m / tc][tr][tc])araw;
	
	const eltype (*const b)[n / tr][tc][tr]
		= (const eltype (*const)[n / tr][tc][tr])braw;

	eltype (*const r)[n / tr][tr][tr]
		= (eltype (*const)[n / tr][tr][tr])rraw;

	for(unsigned i = baserow; i < baserow + l; i += 1)
	for(unsigned j = 0; j < n; j += 1)
	{
		const unsigned it = i / tr;
		const unsigned ito = i % tr;

		const unsigned jt = j / tr;
		const unsigned jto = j % tr;

		double sum = 0;

		for(unsigned k = 0; k < m; k += 1)
		{
			const unsigned kt = k / tc;
			const unsigned kto = k % tc;
			
			const double aik = a[it][kt][ito][kto];
			const double bkj = b[kt][jt][kto][jto]; 

			sum += aik * bkj;
		}

		r[it][jt][ito][jto] = sum;
	}
}

void matrand(
	unsigned seed,
	unsigned abr,
	eltype *const araw,
	const unsigned baserow,
	const unsigned l,
	const unsigned m,
	const unsigned tc)
{
	if(m % tc)
	{
		fail("won't randomize. m: %u is not aligned on tc: %u\n",
			m, tc);
	}

	const unsigned tr = tilecount / tc;

	eltype (*const a)[m / tc][tr][tc] =
		(eltype (*const)[m / tc][tr][tc])araw;

//  	eprintf("randomizing for: a: %p; baserow: %u; l: %u; m: %u; "
// 		"tr: %u; tc: %u\n",
// 		a, baserow, l, m, tr, tc);

	for(unsigned i = baserow; i < baserow + l; i += 1)
	for(unsigned j = 0; j < m; j += 1)
	{
		a[i/tr][j/tc][i % tr][j % tc]
			= 1.0 / (double)((rand_r(&seed) >> 24) + 1);

//		a[i/tr][j/tc][i%tr][j%tc] = (double)(i + abr == j);
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
	const eltype (*const a)[n/tc][tr][tc]
		= (const eltype (*const)[n/tc][tr][tc])araw;

	return a[i/tr][j/tc][i%tr][j%tc];
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
