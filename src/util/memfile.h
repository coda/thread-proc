#ifndef memfilehincluded
#define memfilehincluded

#include <util/config.h>

extern int makeshm(const runconfig *const cfg, const unsigned size);
extern void dropshm(const int);

#endif
