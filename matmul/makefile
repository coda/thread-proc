cc = clang -std=c99 -pedantic -Wall -Werror -flto -O4
cflags = -D_POSIX_C_SOURCE=200809 -D_GNU_SOURCE -I .

tests = mnt mtt mnp mtp mnpf mtpf

all: $(tests)

clean:
	rm *.o $(tests) || true

%.o: %.c
	$(cc) -c $(cflags) $< -o $@

naive = naivemul.o util.o
tile = tilemul.o util.o

mnt: thrd.c $(naive)
	$(cc) $(cflags) $^ -pthread -o $@

mtt: thrd.c $(tile)
	$(cc) $(cflags) $^ -pthread -o $@

mnp: proc.c $(naive)
	$(cc) $(cflags) $^ -o $@

mtp: proc.c $(tile)
	$(cc) $(cflags) $^ -o $@

mnpf: proconfs.c $(naive)
	$(cc) $(cflags) $^ -lrt -o $@

mtpf: proconfs.c $(tile)
	$(cc) $(cflags) $^ -lrt -o $@


naivemul.o: mul.h naivemul.c
util.o: util.h util.c
