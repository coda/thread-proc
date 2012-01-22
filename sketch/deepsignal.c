#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>

static void cancelonfailure(int status, void * arg)
{
	printf("pid %d cancel on failure with status %u is here\n",
		getpid(), status);

	if(status == 0)
	{
		return;
	}
	
	if(kill(-getpgid(getpid()), SIGINT) == 0)
	{
		return;
	}
	
	fprintf(stderr, "cancelling failed\n");
	exit(EXIT_FAILURE);
}

static void waitandcheck(pid_t pid)
{
	int status;
	if(waitpid(pid, &status, 0) == pid)
	{
		if(WIFEXITED(status) && WEXITSTATUS(status) == 0) { } else
		{
			fprintf(stderr, "process %d failed\n", pid);
			exit(EXIT_FAILURE);
		}
	}
	else
	{
		fprintf(stderr, "can't get process %d status\n", pid);
		exit(EXIT_FAILURE);
	}
}

int main(const int argc, const char *const argv[])
{
	pid_t p = fork();

	if(p < 0)
	{
		fprintf(stderr, "1-fork failed\n");
		exit(EXIT_FAILURE);
	}
	else if(p > 0)
	{
		waitandcheck(p);
	}
	else if(p == 0)
	{
		if(setpgid(0, 0) == 0) { } else
		{
			fprintf(stderr, "setting groupleader failed\n");
			exit(EXIT_FAILURE);
		}

		if(on_exit(cancelonfailure, NULL) == 0) { } else
		{
			fprintf(stderr, "can't set on exit handler\n");
			exit(EXIT_FAILURE);
		}

		while(1)
		{
			if(fork() >= 0) { } else
			{
				fprintf(stderr, "FORK FAILED\n");
				exit(EXIT_FAILURE);
			}
		}
	}

	return 0;
}
