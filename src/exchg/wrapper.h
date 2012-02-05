#ifndef wrapperhincluded
#define wrapperhincluded

extern unsigned uintwrite(const int fd, const unsigned i);
extern unsigned uintread(const int fd);
extern void wprclose(const int fd);
extern void wprlength(const int fd);

extern void disablesigpipe(void);

#endif
