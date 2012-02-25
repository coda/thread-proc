#ifndef confighincluded
#define confighincluded

enum { cfghugetlb = 1, cfgaffinegroup = 1 << 1 };

typedef struct {
	unsigned flags;
	unsigned pagelength;
	unsigned size;
	unsigned nworkers;
	unsigned ncores;
	unsigned corelist[1];
} runconfig;

extern runconfig * formconfig(
	const int argc, const char *const argv[],
	const unsigned defaultnworkers,
	const unsigned defaultsize);

extern void freeconfig(runconfig *const rc);

#endif
