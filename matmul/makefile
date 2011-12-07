cc = clang -std=c99 -pedantic -Wall -Werror -flto -O4
cflags = -D_POSIX_C_SOURCE=200809 -I .

all: mnt mnps

clean:
	rm *.o nmt nmp || true

%.o: %.c
	$(cc) -c $(cflags) $< -o $@

naive = naivemul.o util.o

mnt: naivethrd.c $(naive)
	$(cc) $(cflags) $^ -pthread -lrt -o $@

mnps: naiveprocshm.c $(naive)
	$(cc) $(cflags) $^ -lrt -o $@

naivemul.o: mul.h naivemul.c
util.o: util.h util.c
