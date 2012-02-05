#define _POSIX_C_SOURCE 200809
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>

int main(int argc, char ** argv)
{
	const long plen = sysconf(_SC_PAGESIZE);

	char *const ptr = mmap(NULL, 8 * plen, PROT_READ | PROT_WRITE,
		MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);
	if(ptr != MAP_FAILED) {} else { exit(1); }
	printf("mapped\n");

	*ptr = 'x';
	*(ptr + plen) = 'y';

	char *const rptr
		= mremap(ptr + plen, 7 * plen, 7 * plen, MREMAP_MAYMOVE);
	if(rptr != MAP_FAILED) {} else { perror("remap"); exit(1); }
	printf("remapped\n");

	printf("%c %c\n", ptr[0], rptr[0]);

	if(munmap(ptr, plen) == 0) {} else { exit(1); }
	printf("unmapped\n");

	printf("%c\n", rptr[0]);
	printf("%c\n", ptr[0]);

	return 0;
}
