#ifndef wrapperhincluded
#define wrapperhincluded

enum { piperead = 0, pipewrite = 1 };

extern void wprpipe(int pipefds[]);
extern void wprclose(const int fd);
extern void wprtruncate(const int fd, const unsigned len);
extern unsigned wprlength(const int fd);

extern void disablesigpipe(void);

#endif
