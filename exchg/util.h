#ifndef utilhincluded
#define utilhincluded

enum { cfghugetlb = 1, cachelinelength = 64 };

typedef struct
{
	unsigned niterations;
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

#define defpad(n, blk) ( \
	(unsigned)(blk - n % blk) & ((unsigned)-1 + (n % blk == 0)) \
)

typedef struct
{
	int towrite;
	int toread;
//	unsigned listening;
	unsigned nexchanges;
	unsigned writable;
} ringlink;

extern void makerlink(int *const towrite, int *const toread);
extern void droprlink(const ringlink *const rl);

extern unsigned uiwrite(const int fd, const unsigned i);
extern unsigned uiread(const int fd);

extern void ignoresigpipe(void);

#endif
