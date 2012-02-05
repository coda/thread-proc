#ifndef ringlinkhincluded
#define ringlinkhincluded

typedef struct
{
	int readend;
	int writeend;
	unsigned nexchanges;
	unsigned writable;
} ringlink;

extern void rlform(ringlink *const rl, const int readend, const int writeend);
extern void rldrop(ringlink *const rl);

extern void rlwrite(ringlink *const, const unsigned);
extern unsigned rlread(ringlink *const);

#endif
