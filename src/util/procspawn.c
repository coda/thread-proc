#include <./util/spawn.h>
#include <./util/echotwo.h>

#include <stdlib.h>
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
		fail("can't wait up process %d exit", p);
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

static void setaffinity(
	const runconfig *const rc,
	const pid_t p, const unsigned core
) {
	unsigned char coresetbytes[CPU_ALLOC_SIZE(rc->ncores)];
	cpu_set_t *const coreset = (cpu_set_t *)coresetbytes;
	CPU_ZERO_S(sizeof(coreset), coreset);
	CPU_SET_S(core, sizeof(coreset), coreset);

	if(sched_setaffinity(p, sizeof(coreset), coreset) == 0) { } else
	{
		fail("can't set affinity to %u core for proc %d", core, p);
	}
}

static void runnode
(
	const treepreroutine tpr,
	const treeroutine tr,
	const unsigned id,
	const runconfig *const rc,
	void *const prevarg
) {
}

extern void treespawn
(
	treepreroutine tpr, treeroutine tr, const runconfig *const rc
) {
	pid_t p = fork();

	if(p < 0)
	{
		fail("can't fork proc tree controller");
	}
	if(p > 0)
	{
		setaffinity(rc, p, rc->corelist[0]);
		waitsuccess(p);
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

		runnode(tpr, tr, 0, rc, NULL);
		exit(0);
	}
}
