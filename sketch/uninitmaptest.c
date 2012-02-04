#include <sys/mman.h>
#include <linux/mman.h>

#ifndef __linux
#error "not in linux"
#endif

#ifndef MAP_UNINITIALIZED
#error "not supported"
#endif
