cc = clang -std=c99 -pedantic -Wall -Werror -O4 -flto
cflags = -D_POSIX_C_SOURCE=200809 -D_GNU_SOURCE -I .

cxx = clang++ -std=c++0x -pedantic -Wall -Werror -O4 -flto
cxxflags = $(cflags)


tests = et ep
code = util.o 

all: $(tests)

clean:
	rm *.o $(tests) || true

%.o: %.c
	$(cc) -c $(cflags) $< -o $@

%.o: %.cpp
	$(cxx) -c $(cxxflags) $< -o $@

et: thrd.cpp $(code)
	$(cxx) $(cxxflags) $^ -pthread -lrt -o $@

ep: proc.c $(code)
	$(cc) $(cflags) $^ -o $@

util.o: util.c util.h
