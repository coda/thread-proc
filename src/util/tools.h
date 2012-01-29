#ifndef toolshincluded
#define toolshincluded

enum
{
	cachelinelength = 64
};

#define defpad(n, blk) \
( \
	(unsigned)(blk - n % blk) & ((unsigned)-1 + (n % blk == 0)) \
)

extern unsigned align(const unsigned n, const unsigned blocklen);
extern unsigned aligndown(const unsigned n, const unsigned blocklen);

#endif
