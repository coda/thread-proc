#include <matmul/mul.h>
#include <matmul/util.h>

#include <stdlib.h>

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

//	if(l % tr || m % tc || n % tr) l may not be aligned
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
	eltype *const araw,
	const unsigned baserow,
	const unsigned l,
	const unsigned m,
	const unsigned tc)
{
	if(m % tc)
	{
		eprintf("won't randomize. m: %u is not aligned on tc: %u\n",
			m, tc);

		return;
	}

	const unsigned tr = tilesize / (sizeof(eltype) * tc);
	eltype (*const a)[m / tc][tr][tc] =
		(eltype (*const)[m / tc][tr][tc])araw;

	for(unsigned i = baserow; i < baserow + l; i += 1)
	for(unsigned j = 0; j < m; j += 1)
	{
		a[i / tr][j / tc][i % tr][j % tc]
			= 1.0 / (double)((rand_r(&seed) >> 24) + 1);
//		a[i / tr][j / tc][i % tr][j % tc] = (double)i;
	}
}

eltype matat(
	const eltype araw[],
	const unsigned m,
	const unsigned i,
	const unsigned j,
	const unsigned tr,
	const unsigned tc)
{
	if(m % tc)
	{
		eprintf("won't read element. m: %u is not aligned on tc: %u\n",
			m, tc);

		return 0;
	}

//	const unsigned tr = tilesize / (sizeof(eltype) * tc);
	const eltype (*const a)[m / tc][tr][tc]
		= (const eltype (*const)[m / tc][tr][tc])araw;

	return a[i / tr][j / tc][i % tr][j % tc];
}
