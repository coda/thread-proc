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

#endif
