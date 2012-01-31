#include <./util/spawn.h>
#include <./util/echotwo.h>

#include <stdlib.h>

void * makeidargument(
	const treeplugin *const tp,
	const unsigned id, 
	const void *const parentarg)
{
	idargument *const ia = malloc(sizeof(idargument));

	if(ia)
	{
		ia->id = id;
		ia->rc = tp->rc;
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
