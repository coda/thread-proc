#include <exchg/wrapper.h>

#include <echotwo.h>

#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>

void wprpipe(int pipefds[])
{
	if(pipe(pipefds) == 0) { } else
	{
		fail("can't create pipe");
	}
}

void wprclose(const int fd)
{
	if(close(fd) == 0) { } else
	{
		fail("can't close %d", fd);
	}
}

unsigned wprlength(const int fd)
{
	struct stat st;

	if(fstat(fd, &st) == 0) { } else
	{
		fail("can't fstat to get length of %d", fd);
	}

	if(st.st_size < ((unsigned)-1 >> 1)) { } else
	{
		fail("file %d of len %lu is too long",
			fd, (unsigned long)st.st_size);
	}

	return st.st_size;
}

void wprtruncate(const int fd, const unsigned len)
{
	if(ftruncate(fd, len) == 0) { } else
	{
		fail("can't truncate");
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
