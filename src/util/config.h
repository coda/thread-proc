#ifndef confighincluded
#define confighincluded

typedef struct {
	unsigned flags;
	unsigned pagelength;
	unsigned size;
	unsigned nworkers;
	unsigned ncores;
	unsigned corelist[1];
} runconfig;

extern runconfig * formconfig(const int argc, const char *const argv[],
	const unsigned defaultsize, const unsigned defaultnworkers);

extern void freeconfig(runconfig *const rc);

#endif
