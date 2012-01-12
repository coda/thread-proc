#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>

int main(const int argc, const char *const *const argv)
{
	const int plen = getpagesize();
	char *const mem = mmap(NULL, plen,
		PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);

	if(mem != MAP_FAILED) {} else
	{
		fprintf(stderr, "err: %s. remap FAILED; plen: %u\n",
			strerror(errno), plen);
		exit(1);
	}

	const pid_t p = fork();
	if(p == 0)
	{
		sprintf(mem, "hello world");
		exit(0);
	}
	else if(p > 0)
	{
		if(waitpid(p, NULL, 0) == p)
		{
			printf("wait DONE\n");
		}
	}
	else
	{
		printf("fork FAIL\n");
		exit(1);
	}

	printf("%s\n", mem);

	return 0;
}
