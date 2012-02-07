#ifndef vectorhincluded
#define vectorhincluded

#include <exchg/heapsum.h>
#include <util/tools.h>
#include <util/config.h>

typedef struct
{
	int fd;
	unsigned offset; // in bytes
	unsigned length; // of data, the file length will be detected with fstat
} vectorfile;

extern eltype vfelat(const vectorfile *const vf, const unsigned i);

typedef struct
{
	vectorfile vf;
	unsigned char padding[defpad(sizeof(vectorfile), cachelinelength)];
} elvector;

typedef struct
{
	char * ptr;
	unsigned capacity; // overall in bytes
	unsigned offset;
	unsigned length;
} vector;

// returns pointer to the beginning of space added
extern eltype * vectorexpand(
	const runconfig *const rc, vector *const v, const unsigned n);

extern void vectorshrink(const runconfig *const rc, vector *const v);

extern void vectorupload(vector *const v, vectorfile *const vf);

extern void vectordownload(
	const runconfig *const rc, vectorfile *const vf, vector *const v);

#endif
