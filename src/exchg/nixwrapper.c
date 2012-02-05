#include <exchg/wrapper.h>

#include <unistd.h>
#include <signal.h>

void wprpipe(int pipefds[])
{
	if(pipe(pipefds) == 0) { } else
	{
		fail("can't create pipe");
	}
}

void wprclose(cont int fd)
{
	if(close(fd) == 0) { } else
	{
		fail("can't close %d", fd);
	}
}

void disablesigpipe(void)
{
	struct sigaction sa;

	sa.sa_handler = SIG_IGN;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;

	if(sigaction(SIGPIPE, &sa, NULL) == 0) { } else
	{
		fail("can't disable SIGPIPE");
	}
}
