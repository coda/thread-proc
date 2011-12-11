cc = clang -std=c99 -pedantic -Wall -Werror -flto -O4
cflags = -D_POSIX_C_SOURCE=200809 -D_GNU_SOURCE -I .

cxx = clang++ -std=c++11 -pedantic -Wall -Werror -flto -O4
cxxflags = $(cflags)


tests = et ep
code = util.o 

all: $(tests)

clean:
	rm $(code) $(tests) || true

%.o: %.c
	$(cc) -c $(cflags) $< -o $@

%.o: %.cpp
	$(cxx) -c $(cxxflags) $< -o $@

et: thrd.cpp $(code)
	$(cxx) $(cxxflags) $^ -pthread -o $@

ep: proc.c $(code)
	$(cc) $(cflags) $^ -o $@

util.o: util.c util.h
