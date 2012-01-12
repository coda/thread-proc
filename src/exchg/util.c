#include <./util.h>

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>

void eprintf(const char *const format, ...)
{
	va_list al;
	va_start(al, format);
	vfprintf(stderr, format, al);
	va_end(al);
}

void fail(const char *const format, ...)
{
	const char *const errstr = strerror(errno);
	va_list al;
	va_start(al, format);
	vfprintf(stderr, format, al);
	va_end(al);
	eprintf("; err: %s.\n", errstr);
	exit(1);
}

static unsigned ispowerof2(unsigned x)
{
	return (x ^ (x - 1)) + 1 == x << 1;
}

testconfig fillconfig(const unsigned argc,const char *const *const argv)
{
	unsigned flags = 0;

	const long sysplen = sysconf(_SC_PAGESIZE);
	if(sysplen != -1) {} else
	{
		fail("can't get system pagesize");
	}

	const unsigned baseiter = 512;
	unsigned ni = 2 * baseiter;
	unsigned nw = 64;

	if(argc > 1)
	{
		const int i = atoi(argv[1]);
//		ni = i > 0 ? i * baseiter : ni;
		nw = i > 0 ? i : nw;
	}

	if(argc > 2)
	{
		const int i = atoi(argv[2]);
//		nw = i > 0 ? i : nw;
		ni = i > 0 ? i * baseiter : ni;
	}

	unsigned plen = sysplen;
	if(argc > 3)
	{
		const int i = atoi(argv[3]);	
		if(i > 0 && ispowerof2(i) && i > sysplen)
		{
			plen = i;
			flags |= cfghugetlb;
		}
		else
		{
			plen = sysplen;
		}
	}

	printf("config with:\n\tpagelength: %uKiB\n\tflags: %u\n\t"
		"niters: %u\n\tnwrks: %u\n\titers/wrk: %u\n",
		plen / 1024, flags, ni, nw, ni / nw);

	return (testconfig){ 
		.niterations = ni, .nworkers = nw,
		.flags = flags, .pagelength = plen };	
}

// static
unsigned align(const unsigned n, const unsigned blocksize)
{
	unsigned r = n % blocksize;
	unsigned t = (blocksize - r) & ((unsigned)-1 + (r == 0));
	return n + t;
}

// static
unsigned aligndown(const unsigned n, const unsigned blocksize)
{
	return n - n % blocksize;
}
 
static const char * fmtdetect()
{
	switch(sizeof(pid_t))
	{
		case 1: return "/shm-%hhx";
		case 2: return "/shm-%hx";
		case 4: return "/shm-%lx";
		case 8: return "/shm-%llx";
	}

	return "/shm-%x";
}

static const char * htlbfmtdetect()
{
	switch(sizeof(pid_t))
	{
		case 1: return "/tmp/hugetlb/shm-%hhx";
		case 2: return "/tmp/hugetlb/shm-%hx";
		case 4: return "/tmp/hugetlb/shm-%lx";
		case 8: return "/tmp/hugetlb/shm-%llx";
	}

	return "/tmp/hugetlb/shm-%x";
}

int makeshm(const testconfig *const cfg, const unsigned size)
{
	int fd = -1;

	if(!(cfg->flags & cfghugetlb))
	{
		char shmname[strlen("/shm-") + sizeof(pid_t) * 2 + 1];
		sprintf(shmname, fmtdetect(), getpid());

		fd = shm_open(shmname, O_RDWR | O_CREAT, 0600);
		if(fd >= 0) {} else
		{
			fail("can't open shm: %s", shmname);
		}
		if(shm_unlink(shmname) == 0) {} else 
		{
			fail("can't unlink shm: %s", shmname);
		}
	}
	else
	{
		const unsigned namelen = strlen("/tmp/hugetlb/shm-")
			+ sizeof(pid_t) * 2 + 1;
		char htlbname[namelen];
		sprintf(htlbname, htlbfmtdetect(), getpid());

		fd = open(htlbname, O_RDWR | O_CREAT, 0600);
		if(fd >= 0) {} else
		{
			fail("can't open huge tlb: %s", htlbname);
		}
		if(unlink(htlbname) == 0) {} else
		{
			fail("can't unlink huge tlb: %s", htlbname);
		}
	}

	const long plen = cfg->pagelength;

	const unsigned shmlen = align(size, plen);

	if(ftruncate(fd, shmlen) == 0) {} else
	{
		fail("can't resize shm to %u(%u) bytes", shmlen, size);
	}

	return fd;
}


char * peekmap(
	const testconfig *const cfg,
	const int fd,
	const unsigned offset,
	const unsigned length,
	const unsigned pmflags)
{
	const unsigned len = align(length, cfg->pagelength);
	
	unsigned flags = (pmflags & pmprivate) ? MAP_PRIVATE : MAP_SHARED;
	const unsigned prot = (pmflags & pmwrite) ? PROT_WRITE : 0;

	if(fd == -1)
	{
		flags |= MAP_ANONYMOUS; // | MAP_UNINITIALIZED;

		if(cfg->flags & cfghugetlb)
		{
			flags |= MAP_HUGETLB;
		}
	}

	void * m = mmap(NULL, len, PROT_READ | prot, flags, fd, offset);
	if(m != MAP_FAILED) {} else
	{
		fail("peekmap. can't mmap. "
			"pid: %u; len: %u; off: %u; pid: %d\n",
			getpid(), len, offset, fd);
	}

	return m;
}

void dropmap(const testconfig *const cfg, void *const ptr, const unsigned len)
{
	const unsigned l = align(len, cfg->pagelength);
	
	if(munmap(ptr, l) == 0) {} else
	{
		fail("can't unap %p of len %u(%u)", ptr, len, l);
	}
}

enum { piperead = 0, pipewrite = 1 };

void makerlink(int *const towrite, int *const toread)
{
	int fds[2];

	if(pipe(fds) == 0) {} else
	{
		fail("can't make pipe");
	}

	*toread = fds[piperead];
	*towrite = fds[pipewrite];
}

void uclose(const int fd)
{
	if(close(fd) == 0) {} else
	{
		fail("close failure on %d\n", fd);
	}
}

void droprlink(const ringlink *const rl)
{
// 	if(close(rl->toread) || close(rl->towrite))
// 	{
// 		fail("close failure on %d or %d\n", rl->toread, rl->towrite);
// 	}

	uclose(rl->toread);
	uclose(rl->towrite);
}

unsigned uiwrite(const int fd, const unsigned i)
{
	unsigned writable = 1;

	int rv = write(fd, &i, sizeof(i));
	if(rv == sizeof(i))
	{
	}
	else if(errno == EPIPE)
	{
		writable = 0;
	}
	else
	{
// 		fail("can't write to %d; errno: %d; EPIPE: %d",
// 			fd, errno, EPIPE);

		fail("can't write to %d", fd);
	}

	return writable;
}

unsigned uiread(const int fd)
{
	unsigned i;

	int rv = read(fd, &i, sizeof(i));
	if(rv == sizeof(i)) {} else
	{
		fail("can't read from %d; byte: %d; i: %x", fd, rv, i);
	}

	return i;
}

void ignoresigpipe(void)
{
	struct sigaction sa;

	sa.sa_handler = SIG_IGN;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;

	if(sigaction(SIGPIPE, &sa, NULL) == 0) {} else
	{
		fail("can't ignore SIGPIPE");
	}
}

unsigned flength(const int fd)
{
	off_t l = lseek(fd, 0, SEEK_END);
	if(l > 0) {} else
	{
		fail("can't get length of fd:%u", fd);
	}

	if(l < ((unsigned)-1 >> 1)) {} else
	{
		fail("fd:%u too long", fd);
	}

	return l;
}