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

	const unsigned elcount = sysplen / sizeof(eltype);	

	unsigned sz = 2 * elcount;
	unsigned nw = 64;

	if(argc > 1)
	{
		const int i = atoi(argv[1]);
		sz = i > 0 ? i * elcount : sz;
	}

	if(argc > 2)
	{
		const int i = atoi(argv[2]);
		nw = i > 0 ? i : nw;
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

	eprintf("config with "
		"pagelength: %ldKiB; flags: %u; elements per page: %u\n",
		plen / 1024, flags, elcount);

	return (testconfig){ 
		.size = sz, .nworkers = nw,
		.flags = flags, .pagelength = plen };	
}

typedef struct
{
	unsigned startrow;
	unsigned nrows;
} jobitem;

static jobitem ballance(
	const unsigned id,
	const unsigned nwrks,
	const unsigned nrows)
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

static unsigned align(const unsigned n, const unsigned blocksize)
{
	unsigned r = n % blocksize;
	unsigned t = (blocksize - r) & ((unsigned)-1 + (r == 0));
	return n + t;
}

static unsigned aligndown(const unsigned n, const unsigned blocksize)
{
	return n - n % blocksize;
}

joblayout definejob(
	const testconfig *const cfg, const unsigned id,
	const unsigned l, const unsigned m, const unsigned tc)
{
	const unsigned tr = tilesize / (sizeof(eltype) * tc);

	if(m % tc)
	{
		fail("define job. m: %u isn't aligned on tc: %u", m, tc);
	}

	const jobitem ji = ballance(id, cfg->nworkers, l);

	const unsigned nrows = ji.nrows;
	const unsigned abr = aligndown(ji.startrow, tr); // absolute base row
	const unsigned baserow = ji.startrow - abr;

	const unsigned baseoffset = (abr / tr) * sizeof(eltype[m / tc][tr][tc]);

	if(baseoffset != abr * m * sizeof(eltype))
	{
		fail("define job. baseoffset computation isn't correct");
	}

	const unsigned mapoffset = aligndown(baseoffset, cfg->pagelength);

	const unsigned ntilerows = align(baserow + nrows, tr) / tr;
	const unsigned len = ntilerows * sizeof(eltype[m / tc][tr][tc]);
	const unsigned maplength = align(len, cfg->pagelength);

	return (joblayout){
		.nrows = nrows, .baserow = baserow, .baseoffset = baseoffset,
		.mapoffset = mapoffset, .maplength = maplength };
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
// 	if(plen > 0) {} else
// 	{
// 		eprintf("err: %s. can't get page length\n", strerror(errno));
// 		exit(-1);
// 	}

	const unsigned shmlen = align(size, plen);

	if(ftruncate(fd, shmlen) == 0) {} else
	{
		fail("can't resize shm to %u(%u) bytes", shmlen, size);
	}

	return fd;
}

// unsigned getpagelength(void)
// {
// 	long l = sysconf(_SC_PAGESIZE);
// 	if(l > 0 && l < 0x7fffffff) {} else
// 	{
// 		eprintf("err: %s. can't get page length (%ld)\n",
// 			strerror(errno), l);
// 		exit(1);
// 	}
// 
// 	return l;
// }

char * peekmap(
	const testconfig *const cfg,
	const int fd,
	const unsigned offset,
	const unsigned length,
	const unsigned prot)
{
	const unsigned len = align(length, cfg->pagelength);
	
	unsigned flags = MAP_SHARED;

	if(fd == -1)
	{
		flags |= MAP_ANONYMOUS;

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

//	eprintf("peekmap done\n");

	return m;
}
