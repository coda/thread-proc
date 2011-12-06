cc = clang -flto -std=c99 -pedantic -Wall -Werror -O4
cflags = -D_POSIX_C_SOURCE=200809 -I .

all: nmt nmp

clean:
	rm *.o nmt nmp || true

%.o: %.c
	$(cc) -c $(cflags) $< -o $@

naive = naivemul.o util.o

nmt: thrd.c $(naive)
	$(cc) $(cflags) $^ -pthread -o $@

nmp: proc.c $(naive)
	$(cc) $(cflags) $^ -lrt -o $@

naivemul.o: mul.h naivemul.c
util.o: util.h util.c
