#include <./util.h>
#include <./mul.h>

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>


void eprintf(const char *const format, ...)
{
	va_list al;
	va_start (al, format);
	vfprintf(stderr, format, al);
	va_end(al);
}

testconfig fillconfig(
	const unsigned argc,
	const char *const *const argv)
{
	const long plen = sysconf(_SC_PAGESIZE);
	if(plen != -1) {} else
	{
		eprintf("err: %s. can't get pagesize\n", strerror(errno));
		exit(1);
	}

	const unsigned ellen = plen / sizeof(eltype);	

	printf("configuring with pagelength: %ld; elements per page: %u\n",
		plen, ellen);
		
	unsigned sz = 2 * ellen;
	unsigned nw = 64;

	if(argc > 1)
	{
		const int i = atoi(argv[1]);
		sz = i > 0 ? i * ellen : sz;
	}

	if(argc > 2)
	{
		const int i = atoi(argv[2]);
		nw = i > 0 ? i : nw;
	}

	return (testconfig){ .size = sz, .nworkers = nw };	
}

jobitem ballance(const unsigned id, const unsigned nwrks, const unsigned nrows)
{
	unsigned sr = 0;
	unsigned l = 0;

	if(nrows > nwrks)
	{
		const unsigned chunk = nrows / nwrks;

		sr = chunk * id;
		if(id < nwrks - 1) // not the last worker
		{
			l = chunk;
		}
		else // the last one takes the rest
		{
			l = nrows - sr;
		}
	}
	else // one row per worker
	{
		if(id < nrows)
		{
			sr = id;
			l = 1;
		}
	}

	return (jobitem){ .startrow = sr, .nrows = l };
}

unsigned align(const unsigned n, const unsigned blocksize)
{
	unsigned r = n % blocksize;
	unsigned t = (blocksize - r) & ((unsigned)-1 + (r == 0));
	return n + t;
}

unsigned aligndown(const unsigned n, const unsigned blocksize)
{
	return n - n % blocksize;
}

static const char * fmtdetect()
{
	switch(sizeof(pid_t))
	{
		case 1: return "rmf.shm-%hhx";
		case 2: return "rmf.shm-%hx";
		case 4: return "rmf.shm-%lx";
		case 8: return "rmf.shm-%llx";
	}

	return "rmf.shm-%x";
}

int makeshm(const unsigned size)
{
	char shmname[strlen("/rmf.shm-") + sizeof(pid_t) * 2 + 1];
	sprintf(shmname, fmtdetect(), getpid());

	int fd = shm_open(shmname, O_RDWR | O_CREAT, 0600);
	if(fd >= 0) {} else
	{
		eprintf("err: %s. can't open shm: %s\n",
			strerror(errno), shmname);
		exit(1);
	}
	if(shm_unlink(shmname) == 0) {} else 
	{
		eprintf("err: %s. can't unlink name: %s\n",
			strerror(errno), shmname);
		exit(1);
	}

	const long plen = sysconf(_SC_PAGESIZE);
	if(plen > 0) {} else
	{
		eprintf("err: %s. can't get page length\n", strerror(errno));
		exit(-1);
	}

	const unsigned shmlen = align(size, plen);

	if(ftruncate(fd, shmlen) == 0) {} else
	{
		eprintf("err: %s. can't resize shm to %u(%u) bytes\n",
			strerror(errno), shmlen, size);
		exit(-1);
	}

	return fd;
}

unsigned getpagelength(void)
{
	long l = sysconf(_SC_PAGESIZE);
	if(l > 0 && l < 0x7fffffff) {} else
	{
		eprintf("err: %s. can't get page length (%ld)\n",
			strerror(errno), l);
		exit(1);
	}

	return l;
}
