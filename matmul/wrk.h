#ifndef wrkhincluded
#define wrkhincluded

extern void work
(
	const double *const amtx,
	const double *const bmtx,
	const unsigned l,
	const unsigned m,
	const unsigned n,
	const unsigned wrkcount,
	double *const rmtx
);

#endif
