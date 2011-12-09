cc = clang -std=c99 -pedantic -Wall -Werror -flto -O4
cflags = -D_POSIX_C_SOURCE=200809 -D_GNU_SOURCE -I .

tests = mnt mnp mnpf 

all: $(tests)

clean:
	rm *.o $(tests) || true

%.o: %.c
	$(cc) -c $(cflags) $< -o $@

naive = naivemul.o util.o

mnt: naivethrd.c $(naive)
	$(cc) $(cflags) $^ -pthread -o $@

mnpf: naiveproconfs.c $(naive)
	$(cc) $(cflags) $^ -lrt -o $@

mnp: naiveproc.c $(naive)
	$(cc) $(cflags) $^ -o $@

naivemul.o: mul.h naivemul.c
util.o: util.h util.c
