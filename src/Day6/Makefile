FLAGS=-std=c++17 -march=native -O3 -g
FLAGS_DEBUG=-std=c++17 -march=native -g
LIBS=-ltbb -lfmt
INCLUDE=-I/usr/include/eigen3

all:
	g++ ${FLAGS} ${LIBS} ${INCLUDE} day6.cpp -ltbb -lfmt

debug:
	g++ ${FLAGS_DEBUG} ${LIBS} ${INCLUDE} day6.cpp -ltbb -lfmt

clean:
	rm a.out
