#ifndef memmaphincluded
#define memmaphincluded

#include <util/config.h>

enum
{
	pmabwrite = 0x01
};

enum
{
	pmcfgshared = 0x01
};

extern void * peekmap(
	const runconfig *const cfg,
	const int fd,
	const unsigned len,
	const unsigned offset,
	const unsigned accessbits,
	const unsigned configbits);

extern void dropmap(
	const runconfig *const rc, void *const ptr, const unsigned len);

#endif
