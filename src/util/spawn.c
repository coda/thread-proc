#include <./util/spawn.h>

void * makeidargument(const unsigned id, const runconfig *const rc,
	const void *const parentarg)
{
	return (void *)id;
}

void dropidargument(void *const arg)
{
}
