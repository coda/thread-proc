#ifndef utilhincluded
#define utilhincluded

typedef struct
{
	unsigned size;
	unsigned nworkers;
} testconfig;

extern void eprintf(const char *const, ...);
extern testconfig fillconfig(const unsigned, const char *const *const);

#endif
