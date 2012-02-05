#ifndef memmaphincluded
#define memmaphincluded

#include <util/config.h>

enum { pmwrite = 0x01, pmshared = 0x02 };

extern void * peekmap(
	const runconfig *const cfg,
	const int fd,
	const unsigned len,
	const unsigned offset,
	const unsigned configbits);

extern void dropmap(
	const runconfig *const rc, void *const ptr, const unsigned len);

#endif
