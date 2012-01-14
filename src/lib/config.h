#ifndef confighincluded
#define confighincluded

typedef struct {
	unsigned flags;
	int pagelength;
	unsigned size;
	unsigned nworkers;
	unsigned ncpu;
	unsigned cpulist[0];
} runconfig;

extern runconfig * formconfig(const int argc, const char *const *const argv,
	const unsigned defaultsize, const unsigned defaultnworkers);

extern void freeconfig(runconfig *const rc);

#endif
