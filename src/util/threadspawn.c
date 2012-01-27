#include <./util/spawn.h>
#include <./util/echotwo.h>

#include <stdlib.h>
#include <stdio.h>

#include <pthread.h>

static void joinsuccess(const pthread_t t)
{
	if(pthread_join(t, NULL) == 0) { } else
	{
		fail("can't join thread %d", t);
	}
}

static void setaffinity(const pthread_t t, const runconfig *const rc,
	const unsigned id)
{
	unsigned char coresetbytes[CPU_ALLOC_SIZE(rc->ncores)];
	cpu_set_t *const coreset = (cpu_set_t *)coresetbytes;
	CPU_ZERO_S(sizeof(coreset), coreset);
	CPU_SET_S(rc->corelist[id % rc->ncores], sizeof(coreset), coreset);

	if(pthread_setaffinity_np(t, sizeof(coreset), coreset) == 0) { } else
	{
		fail("can't set %u core affinity for thread %d",
			rc->corelist[id % rc->ncores], t);
	}
}

static pthread_t runthread(void * (*const fnptr)(void *), const void *const arg,
	char onerror[])
{
	pthread_t t;
	if(pthread_create(&t, NULL, fnptr, (void *)arg) == 0) { } else
	{
//		fail("can't run thread for job %u", args[i].id);
		fail(onerror);
	}

	return t;
}

typedef struct
{
	treepreroutine tpr;
	treeroutine tr;
	const runconfig * rc;
} treesetup;

typedef struct
{
	const treesetup * ts;	
	unsigned id;
	void * parentarg;
} threadargument;

static void * threadroutine(void * varg)
{
	const threadargument ta = *(threadargument *)varg;

	void *const arg = ta.ts->tpr(ta.id, ta.ts->rc, ta.parentarg);

	pthread_t threads[2] = { -1, -1 };
	const threadargument args[2] =
	{
		{ .ts = ta.ts, .id = 2 * ta.id + 1, .parentarg = arg },
		{ .ts = ta.ts, .id = 2 * ta.id + 2, .parentarg = arg }
	};

	for(unsigned i = 0; i < 2; i += 1)
	{
		if(args[i].id < ta.ts->rc->nworkers)
		{
			char msg[64];
			sprintf(msg, "can't start job %u", args[i].id);
			threads[i] = runthread(threadroutine, &args[i], msg);
			setaffinity(threads[i], ta.ts->rc, args[i].id);
		}
	}

	ta.ts->tr(arg);
	// free(arg);

	for(unsigned i = 0; i < 2; i += 1)
	{
		if(threads[i] != -1)
		{
//			eprintf("joining %x\n", threads[i]);
			joinsuccess(threads[i]);
		}
	}

	return NULL;
}

void treespawn(const treepreroutine tpr, const treeroutine tr, 
	const runconfig *const rc)
{
	const treesetup ts = { .rc = rc, .tr = tr, .tpr = tpr };
	const threadargument ta =
	{
		.ts = &ts,
		.id = 0,
		.parentarg = tpr(0, rc, NULL)
	};

	const pthread_t t = runthread(threadroutine, &ta, "can't run root job");
	setaffinity(t, rc, 0);
	joinsuccess(t);
	eprintf("thread tree DONE\n");
}
