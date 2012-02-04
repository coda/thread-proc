#include <matmul/mul.h>

#include <stdlib.h>

eltype elidentity(const unsigned i, const unsigned j, unsigned *const state)
{
	return (eltype)(i == j);
}

eltype elrand( const unsigned i, const unsigned j, unsigned *const state)
{
	return (eltype)(1.0 / (eltype)((rand_r(state) >> 24) + 1));
}
