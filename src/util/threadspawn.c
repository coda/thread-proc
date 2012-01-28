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

static pthread_t runthread(void * (*const fnptr)(void *),
	const void *const arg, char onerror[])
{
	pthread_t t;
	if(pthread_create(&t, NULL, fnptr, (void *)arg) == 0) { } else
	{
//		fail("can't run thread for job %u", args[i].id);
		fail(onerror);
	}

	return t;
}

// typedef struct
// {
// 	treepreroutine tpr;
// 	treeroutine tr;
// 	const runconfig * rc;
// } treesetup;

typedef struct
{
	const treeplugin *const tp;
	const runconfig *const rc;
	unsigned id;
	void * parentarg;
} threadargument;

static void * threadroutine(void * varg)
{
	const threadargument ta = *(threadargument *)varg;

	void *const arg = ta.tp->makeargument(ta.id, ta.rc, ta.parentarg);

	pthread_t threads[2] = { -1, -1 };
	const threadargument args[2] =
	{
		{ .tp = ta.tp, .rc = ta.rc,
			.id = 2 * ta.id + 1, .parentarg = arg },

		{ .tp = ta.tp, .rc = ta.rc,
			.id = 2 * ta.id + 2, .parentarg = arg }
	};

	for(unsigned i = 0; i < 2; i += 1)
	{
		if(args[i].id < ta.rc->nworkers)
		{
			char msg[64];
			sprintf(msg, "can't start job %u", args[i].id);
			threads[i] = runthread(threadroutine, &args[i], msg);
			setaffinity(threads[i], ta.rc, args[i].id);
		}
	}

	ta.tp->treeroutine(arg);

	for(unsigned i = 0; i < 2; i += 1)
	{
		if(threads[i] != -1)
		{
//			eprintf("joining %x\n", threads[i]);
			joinsuccess(threads[i]);
		}
	}

	ta.tp->dropargument(arg);

	return NULL;
}

void treespawn(const treeplugin *const tp, const runconfig *const rc)
{
	const threadargument ta =
	{
		.tp = tp,
		.rc = rc,
		.id = 0,
		.parentarg = tp->makeargument(0, rc, NULL)
	};

	const pthread_t t = runthread(threadroutine, &ta, "can't run root job");
	setaffinity(t, rc, 0);
	joinsuccess(t);
	tp->dropargument(ta.parentarg);
	eprintf("thread tree DONE\n");
}
