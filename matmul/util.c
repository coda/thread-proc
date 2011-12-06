#include <./util.h>
#include <./mul.h>

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

#include <errno.h>
#include <unistd.h>


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

	eprintf("configuring with pagelength: %u; elements per page: %u\n",
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
