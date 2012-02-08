#include <util/memmap.h>
#include <util/tools.h>
#include <util/echotwo.h>

#include <stdlib.h>

#include <sys/mman.h>
#ifdef __linux
#include <linux/mman.h>
#endif

void * peekmap(
	const runconfig *const rc,
	const int fd,
	const unsigned offset,
	const unsigned length,
	const unsigned config)
{
	const unsigned len = align(length, rc->pagelength);
	const unsigned prot = (config & pmwrite) ? PROT_WRITE : 0;
	unsigned flags = (config & pmshared) ? MAP_SHARED : MAP_PRIVATE;

	if(fd == -1)
	{
#ifdef MAP_ANONYMOUS
		flags |= MAP_ANONYMOUS;
#else
		fail("anonymouse mmap unsopported");
#endif

#ifdef MAP_UNINITIALIZED
		flags |= MAP_UNINITIALIZED;
#else
		eprintf("unitinialized mmap unsupported\n");
#endif

		if(rc->flags & cfghugetlb)
		{
#ifdef MAP_HUGETLB
			flags |= MAP_HUGETLB;
#else
			eprintf("hugetlb mmap unsupported\n");
#endif
		}
	}

	void * m = mmap(NULL, len, PROT_READ | prot, flags, fd, offset);
	if(m != MAP_FAILED) { } else
	{
		fail("peekmap. can't mmap. len: %u; off: %u; fd: %d",
			len, offset, fd);
	}

	return m;
}

void dropmap(const runconfig *const rc, void *const ptr, const unsigned len)
{
	const size_t plen = align(len, rc->pagelength);
	if(munmap(ptr, plen) == 0) { } else
	{
		fail("can't unmap %p of length: %u", ptr, len);
	}
}
