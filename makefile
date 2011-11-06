cc = clang -std=c99 -Wall -Werror -O4 -D_POSIX_SOURCE
cflags = -I .

all: tt pt

clean:
	@ rm *.o tt pt || true

%.o: %.c
	@ $(cc) -c $(cflags) $< -o $@

tt: thrdtest.c worker.o
	@ $(cc) $(cflags) $^ -pthread -o $@

pt: proctest.c worker.o
	@ $(cc) $(cflags) $^ -o $@

worker.o: worker.c worker.h
