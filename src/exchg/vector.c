#include <exchg/vector.h>
#include <exchg/wrapper.h>
#include <util/echotwo.h>
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

eltype * vectorexpand(
	const runconfig *const rc, vector *const v, const unsigned n)
{
	const unsigned plen = rc->pagelength;
	const unsigned need = v->offset + v->length + n * sizeof(eltype);

	if(need <= v->capacity)
	{
		return (eltype *)(v->ptr + v->offset + v->length);
	}

	void *const ptr
		= mremap(v->ptr, v->capacity,
			align(need, plen), MREMAP_MAYMOVE);

	if(ptr != MAP_FAILED) { } else
	{
		fail("expanding. can't remap");
	}
	
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

	if(v->ptr && v->capacity)
	{
		if(pwrite(vf->fd,
			v->ptr, v->capacity, 0) == v->capacity) { } else
		{
			fail("uploading. can't write old data");
		}
	}

	wprtruncate(vf->fd, v->capacity);
}

void vectordownload(
	const runconfig *const rc, vectorfile *const vf, vector *const v)
{
	const unsigned len = wprlength(vf->fd);

	if(len <= v->capacity) { } else
	{
		void *const ptr
			= mremap(v->ptr, v->capacity, len, MREMAP_MAYMOVE);

		if(ptr != MAP_FAILED) { } else
		{
			fail("reloading. can't remap");
		}

		v->ptr = ptr;
		v->capacity = len;
	}

	if(pread(vf->fd, v->ptr, len, 0) == len) { } else
	{
		fail("downloading. can't get new vector");
	}

	v->offset = vf->offset;
	v->length = vf->length;
}
