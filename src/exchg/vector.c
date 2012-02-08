#include <exchg/vector.h>
#include <exchg/wrapper.h>
#include <util/echotwo.h>
#include <util/memmap.h>
#include <unistd.h>
#include <sys/mman.h>

eltype vfelat(const vectorfile *const vf, const unsigned i)
{
	const unsigned elcnt = vf->length / sizeof(eltype);

	if(i < elcnt) { } else
	{
		fail("idx: %i is out of bounds", i);
	}

	off_t pos = vf->offset + i*sizeof(eltype);
	if(lseek(vf->fd, pos, SEEK_SET) == pos) { } else
	{
		fail("can't seek to pos:%u = off:%u + i:%u",
			pos, vf->offset, i * sizeof(eltype));
	}

	eltype value;
	if(read(vf->fd, &value, sizeof(value)) == sizeof(value)) { } else
	{
		fail("can't read");
	}

	return value;
}

void edumpvector(const vector *const v)
{
	eprintf("v: %p:%u %u %u:%u ",
		v->ptr, v->capacity, v->remapped, v->offset, v->length);
}

eltype * vectorexpand(
	const runconfig *const rc, vector *const v, const unsigned n)
{
	const unsigned plen = rc->pagelength;
	const unsigned need = v->offset + v->length + n * sizeof(eltype);

	if(need < v->capacity)
	{
		return (eltype *)(v->ptr + v->offset + v->length);
	}

	void * ptr;
	if(v->capacity)
	{
		ptr = mremap(v->ptr, v->capacity,
			align(need, plen), MREMAP_MAYMOVE);

		if(ptr != MAP_FAILED) { } else
		{
			fail("expanding. can't remap");
		}
	}
	else
	{
		ptr = peekmap(rc, -1, 0, align(need, plen), pmwrite);
	}
	
	v->remapped = 1;
	v->capacity = align(need, plen);
	v->ptr = ptr;

	return (eltype *)(v->ptr + v->offset + v->length);
}

void vectorshrink(const runconfig *const rc, vector *const v)
{
	const unsigned plen = rc->pagelength;
	const unsigned remapoff = aligndown(v->offset, plen);

	if(remapoff > 0)
	{
		if(munmap(v->ptr, remapoff) == 0) { } else
		{
			fail("shrinking. can't unmap");
		}	

		v->ptr += remapoff;
		v->offset -= remapoff;
		v->capacity -= remapoff;
	}
}

void vectorupload(
	const runconfig *const rc, vector *const v, vectorfile *const vf)
{
	vf->offset = v->offset;
	vf->length = v->length;

	if(v->remapped)
	{
		if(pwrite(vf->fd, v->ptr, v->capacity, 0) == v->capacity)
		{
		}
		else
		{
			fail("uploading. can't write old data");
		}
	}

	if(v->ptr && v->capacity)
	{
		dropmap(rc, v->ptr, v->capacity);
	}

	v->ptr = NULL;
	v->offset = 0;
	v->length = 0;
	v->capacity = 0;
	v->remapped = 0;
}

void vectordownload(
	const runconfig *const rc, vectorfile *const vf, vector *const v)
{
	const unsigned len = wprlength(vf->fd);

	if(len)
	{
		v->ptr = peekmap(rc, -1, 0, len, pmwrite);
		v->capacity = len;
		v->remapped = 0;
		v->offset = vf->offset;
		v->length = vf->length;
	}
	else
	{
		v->ptr = NULL;
		v->capacity = len;
		v->offset = 0;
		v->length = 0;
		v->remapped = 0;
	}
}
