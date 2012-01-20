#include <unistd.h>
#include <signal.h>

#include <stdio.h>
#include <stdlib.h>

static void handler(int i)
{
	printf("signal on %d\n", getpid());
	signal(SIGCHLD, handler);
}

int main(int argc, char * argv[])
{
	pid_t p = fork();

	signal(SIGCHLD, handler);

	if(p > 0)
	{
		int i = 10;
		while(i -= 1)
		{
			printf("r-tick\n");
			sleep(1);
		}
	}
	else if(p == 0)
	{
		p = fork();
		if(p == 0)
		{
			sleep(1);
			printf("fork-2 exiting\n");
			exit(0);
		}
		else if(p > 0)
		{
			int i = 5;
			while(i -= 1)
			{
				printf("1-tick\n");
				sleep(1);
			}
		}
		else
		{
			fprintf(stderr, "fork-2\n");
			exit(1);
		}

		printf("fork exiting\n");
		exit(0);
	}
	else
	{
		fprintf(stderr, "fork\n");
		exit(1);
	}

	printf("root exiting\n");

	return 0;
}
