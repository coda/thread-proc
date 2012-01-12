#ifndef workerhincluded
#define workerhincluded

typedef struct rnodetag
{
	struct rnodetag* next;
	struct rnodetag* prev;
} rnode;

enum { cachelinelength = 64 };

#define defpad(n, blk) ( \
	(unsigned)(blk - n % blk) & ((unsigned)-1 + (n % blk == 0)) \
)

typedef struct
{
	rnode * r;
	unsigned char padding[defpad(sizeof(rnode *), cachelinelength)];
} rnodeline;

extern int worker(rnodeline rings[], unsigned id);
extern void freerings(rnodeline rings[]);

typedef struct
{
	unsigned nworkers;
	unsigned niterations;
} configuration;

extern const unsigned nrings;
extern configuration cfg;

extern void fillconfig(unsigned argc, char** argv);

#endif
