FLAGS=-std=c++17 -march=native -O3 -g
FLAGS_DEBUG=-std=c++17 -march=native -g
LIBS=-ltbb -lfmt
INCLUDE=-I/usr/include/eigen3 -I../ -I ../../thirdparty
COMPILER=g++

all:
	${COMPILER} ${FLAGS} ${LIBS} ${INCLUDE} main.cpp	

debug:
	${COMPILER} ${FLAGS_DEBUG} ${LIBS} ${INCLUDE} main.cpp	

clean:
	rm a.out
