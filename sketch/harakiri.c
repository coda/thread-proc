#include <unistd.h>
#include <signal.h>

#include <stdio.h>

int main(const int argc, const char *const argv[])
{
	pid_t p = fork();
	if(p == 0)
	{
		while(1)
		{
			sleep(1);
			fprintf(stderr, "TICK\n");
		}
	}
	else if(p > 0)
	{
		sleep(5);
		fprintf(stderr, "bye-bye");
		kill(-getpid(), SIGINT);
	}

	return 0;
}
