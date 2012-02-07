#ifndef heapsumhincluded
#define heapsumhincluded

typedef double eltype;

enum { workfactor = 1 << 8 };

extern eltype heapsum(eltype *const, const unsigned n);
extern eltype elrand(unsigned *const seed);

#endif
