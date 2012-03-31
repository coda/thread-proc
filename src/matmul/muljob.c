#include <matmul/muljob.h>
#include <matmul/mul.h>
#include <util/tools.h>

#include <echotwo.h>

typedef struct
{
	unsigned startrow;
	unsigned nrows;
} jobitem;

static jobitem ballance(
	const unsigned id,
	const unsigned nwrks,
	const unsigned nrows)
{
	unsigned sr = 0;
	unsigned l = 0;

	if(nrows > nwrks)
	{
		const unsigned chunk = nrows / nwrks;

		sr = chunk * id;
		if(id < nwrks - 1) // not the last worker
		{
			l = chunk;
		}
		else // the last one takes the rest
		{
			l = nrows - sr;
		}
	}
	else // one row per worker
	{
		if(id < nrows)
		{
			sr = id;
			l = 1;
		}
	}

	return (jobitem){ .startrow = sr, .nrows = l };
}

joblayout definejob(
	const runconfig *const cfg,
	const unsigned id,
	const unsigned l, const unsigned m,
	const unsigned tr, const unsigned tc)
{
	if(m % tc)
	{
		fail("define job. m: %u isn't aligned on tc: %u", m, tc);
	}

	const jobitem ji = ballance(id, cfg->nworkers, l);

	const unsigned nrows = ji.nrows;
	const unsigned abr = aligndown(ji.startrow, tr); // absolute base row
	const unsigned baserow = ji.startrow - abr;

	const unsigned baseoffset = (abr / tr) * sizeof(eltype[m / tc][tr][tc]);

	if(baseoffset != abr * m * sizeof(eltype))
	{
		fail("define job. baseoffset computation isn't correct");
	}

	const unsigned mapoffset = aligndown(baseoffset, cfg->pagelength);
	const unsigned odiff = baseoffset - mapoffset;

	const unsigned ntilerows = align(baserow + nrows, tr) / tr;
	const unsigned len = ntilerows * sizeof(eltype[m / tc][tr][tc]);
	const unsigned maplength = align(odiff + len, cfg->pagelength);

	return (joblayout){
		.nrows = nrows, .baserow = baserow, .baseoffset = baseoffset,
		.mapoffset = mapoffset, .maplength = maplength,
		.absolutebaserow = ji.startrow };
}
