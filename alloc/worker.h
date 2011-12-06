#ifndef workerhincluded
#define workerhincluded

typedef struct rnodetag
{
	struct rnodetag* next;
	struct rnodetag* prev;
} rnode;

extern int worker(rnode* rings[], unsigned id);
extern void freerings(rnode* rings[]);

typedef struct
{
	unsigned nworkers;
	unsigned niterations;
} configuration;

extern const unsigned nrings;
extern configuration cfg;

extern void fillconfig(unsigned argc, char** argv);

#endif
