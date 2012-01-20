#include <./util/echotwo.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

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
	eprintf("%d. err: %s. ", getpid(), errstr);	

	va_list al;
	va_start(al, format);
	vfprintf(stderr, format, al);
	va_end(al);

	fputc('\n', stderr);

	exit(EXIT_FAILURE);
}
