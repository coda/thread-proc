#include <util/memfile.h>
#include <util/tools.h>

#include <echotwo.h>

#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#define htlb(x) "/tmp/hugetlb/shm-" #x
#define shm(x) "/shm-" #x

static const char * fmtdetect()
{
	switch(sizeof(pid_t))
	{
		case 1: return shm(%hhx);
		case 2: return shm(%hx);
		case 4: return shm(%lx);
		case 8: return shm(%llx);
	}

	return shm(%x);
}

static const char * htlbfmtdetect()
{
	switch(sizeof(pid_t))
	{
		case 1: return htlb(%hhx);
		case 2: return htlb(%hx);
		case 4: return htlb(%lx);
		case 8: return htlb(%llx);
	}

	return htlb(%x);
}

int makeshm(const runconfig *const rc, const unsigned size)
{
	int fd = -1;

	if(!(rc->flags & cfghugetlb))
	{
		char shmname[strlen(shm()) + sizeof(pid_t) * 2 + 1];
		sprintf(shmname, fmtdetect(), getpid());

		fd = shm_open(shmname, O_RDWR | O_CREAT, 0600);
		if(fd >= 0) { } else
		{
			fail("can't open shm: %s", shmname);
		}

		if(shm_unlink(shmname) == 0) { } else 
		{
			fail("can't unlink shm: %s", shmname);
		}
	}
	else
	{
		char htlbname[strlen(htlb()) + 2 * sizeof(pid_t) + 1];
		sprintf(htlbname, htlbfmtdetect(), getpid());

		fd = open(htlbname, O_RDWR | O_CREAT, 0600);
		if(fd >= 0) { } else
		{
			fail("can't open huge tlb: %s", htlbname);
		}

		if(unlink(htlbname) == 0) { } else
		{
			fail("can't unlink huge tlb: %s", htlbname);
		}
	}

	const long plen = rc->pagelength;
	const unsigned shmlen = align(size, plen);

	if(ftruncate(fd, shmlen) == 0) {} else
	{
		fail("can't resize shm to %u(%u) bytes", shmlen, size);
	}

	return fd;
}

void dropshm(const int fd)
{
	if(close(fd) == 0) { } else
	{
		fail("can't close %d", fd);
	}
}
