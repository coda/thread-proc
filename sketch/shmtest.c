#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <sys/shm.h>

int main(const int argc, const char *const *const argv)
{
	const long plen = sysconf(_SC_PAGESIZE);
	if(plen > 0) {} else
	{
		printf("pagesize FAIL\n");
		exit(1);
	}

	const int shmid = shmget(IPC_PRIVATE, plen, SHM_R | SHM_W);
	if(shmid > 0) {} else
	{
		printf("shmget FAIL\n");
		exit(1);
	}

	void *const mem = shmat(shmid, NULL, 0);
	if(mem != (void *)-1) {} else
	{
		printf("shmat FAIL. err: %s\n", strerror(errno));
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
