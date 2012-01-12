#ifndef exchghincluded
#define exchghincluded

typedef double eltype;

enum { workfactor = 1 << 10 };

extern eltype heapsum(eltype *const, const unsigned n);
extern eltype elrand(unsigned *const seed);

#endif
