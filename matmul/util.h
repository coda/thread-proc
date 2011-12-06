#ifndef utilhincluded
#define utilhincluded

typedef struct
{
	unsigned size;
	unsigned nworkers;
} testconfig;

extern void eprintf(const char *const, ...);

extern testconfig fillconfig(const unsigned, const char *const *const);

extern int makeshm(const unsigned size);

typedef struct
{
	unsigned startrow;
	unsigned nrows;
} jobitem;

extern jobitem ballance(const unsigned, const unsigned, const unsigned);

extern unsigned align(const unsigned, const unsigned);
extern unsigned aligndown(const unsigned, const unsigned);

extern unsigned getpagelength(void);

#endif
