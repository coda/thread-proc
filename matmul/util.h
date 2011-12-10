#ifndef utilhincluded
#define utilhincluded

enum { cfghugetlb = 1 };

typedef struct
{
	unsigned size;
	unsigned nworkers;
	unsigned flags;
	int pagelength;
} testconfig;

extern testconfig fillconfig(const unsigned, const char *const *const);

extern void fail(const char *const, ...);
extern void eprintf(const char *const, ...);

extern int makeshm(const testconfig *const cfg, const unsigned size);
extern char * peekmap(const testconfig *const cfg,
	const int fd, const unsigned len, const unsigned offset,
	const unsigned flag);

// typedef struct
// {
// 	unsigned startrow;
// 	unsigned nrows;
// } jobitem;

// extern jobitem ballance(const unsigned, const unsigned, const unsigned);
// 
// extern unsigned align(const unsigned, const unsigned);
// extern unsigned aligndown(const unsigned, const unsigned);

// extern unsigned getpagelength(void);

typedef struct
{
	unsigned mapoffset;
	unsigned maplength;
	unsigned baseoffset;
	unsigned baserow;
	unsigned nrows;
} joblayout;

extern joblayout layjob(
	const testconfig *const cfg, const unsigned id, const unsigned m);

#endif
