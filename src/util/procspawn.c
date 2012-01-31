#include <./util/spawn.h>
#include <./util/echotwo.h>

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sched.h>
#include <sys/wait.h>

static void waitsuccess(const pid_t p)
{
	int status;
	if(waitpid(p, &status, 0) == p)
	{
		if(WIFEXITED(status) && WEXITSTATUS(status) == 0) { } else
		{
			fail("process %d failed with status: %d", p, status);
		}
	}
	else
	{
		fail("wait for %d 0-exit is failed", p);
	}
}

static void cancelonfail(int status, void * arg)
{
	if(status == EXIT_SUCCESS)
	{
		return;
	}

	if(kill(-getpgid(getpid()), SIGINT) == 0)
	{
		return;
	}

	fail("can't send signal to tree proc group");
}

static void setaffinity(const pid_t p, const runconfig *const rc,
	const unsigned id)
{
	unsigned char coresetbytes[CPU_ALLOC_SIZE(rc->ncores)];
	cpu_set_t *const coreset = (cpu_set_t *)coresetbytes;
	CPU_ZERO_S(sizeof(coreset), coreset);
	CPU_SET_S(rc->corelist[id % rc->ncores], sizeof(coreset), coreset);

	if(sched_setaffinity(p, sizeof(coreset), coreset) == 0) { } else
	{
		fail("can't set affinity to %u core for proc %d",
			rc->corelist[id % rc->ncores], p);
	}
}

static pid_t forkf(char onerror[])
{
	const pid_t p = fork();

	if(p < 0)
	{
		fail(onerror);
	}

	return p;
}

static void runnode(
	const treeplugin *const tp,
	const unsigned id,
	void *const parentarg);

void treespawn(const treeplugin *const tp)
{
	pid_t p = forkf("can't fork proc tree root");

	const runconfig *const rc = tp->rc;

	if(p > 0)
	{
		setaffinity(p, rc, 0);
		waitsuccess(p);
		printf("proc tree DONE\n");
	}
	else
	{
		if(setpgid(0, 0) == 0) { } else
		{
			fail("can't set proc tree group");
		}

		if(on_exit(cancelonfail, NULL) == 0) { } else
		{
			fail("can't set exit on fail handler");
		}

		runnode(tp, 0, NULL);
		exit(0);
	}
}

static void runnode(
	const treeplugin *const tp,
	const unsigned id,
	void *const parentarg)
{
	const runconfig *const rc = tp->rc;

	void *const arg = tp->makeargument(tp, id, parentarg);
	tp->dropargument(parentarg);

	pid_t pids[2] = { -1, -1 };
	const unsigned ids[2] = { 2*id + 1, 2*id + 2 };

	for(unsigned i = 0; i < 2; i += 1)
	{
		if(ids[i] < rc->nworkers)
		{
			char msg[64];
			sprintf(msg, "can't fork job %u", ids[i]);

			pid_t p = forkf(msg);

			if(p > 0)
			{
				setaffinity(p, rc, ids[i]);
				pids[i] = p;
			}
			else
			{
				runnode(tp, ids[i], arg);
				exit(0);
			}
		}
	}

	tp->treeroutine(arg);
	tp->dropargument(arg);

	for(unsigned i = 0; i < 2; i += 1)
	{
		const pid_t p = pids[i];

		if(p > 0)
		{
			waitsuccess(p);
		}
	}
}
