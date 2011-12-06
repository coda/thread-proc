#ifndef utilhincluded
#define utilhincluded

typedef struct
{
	unsigned size;
	unsigned nworkers;
} testconfig;

extern void eprintf(const char *const, ...);
extern void fillconfig(testconfig *const, unsigned, char **);

#endif
