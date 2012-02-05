#ifndef wrapperhincluded
#define wrapperhincluded

enum ( piperead = 0, pipewrite = 1 };

extern void wprpipe(int pipefds[]);
extern void wprclose(const int fd);
extern void wprlength(const int fd);

extern void disablesigpipe(void);

#endif
