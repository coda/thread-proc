#include <./util/spawn.h>
#include <./util/echotwo.h>

#include <stdlib.h>

void * makeidargument(const unsigned id, const runconfig *const rc,
	const void *const parentarg)
{
	idargument *const ia = malloc(sizeof(idargument));
	if(ia)
	{
	}
	else
	{
		fail("can't allocate argument for job %u", id);
	}

	return ia;
}

void dropidargument(void *const arg)
{
	free(arg);
}
