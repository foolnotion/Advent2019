FLAGS=-std=c++17 -march=native -O3 -DNDEBUG -g
FLAGS_DEBUG=-std=c++17 -march=native -g
LIBS=-ltbb -lfmt
INCLUDE=-I/usr/include/eigen3 -I../ -I ../../thirdparty

all:
	g++ ${FLAGS} ${LIBS} ${INCLUDE} main.cpp	

debug:
	g++ ${FLAGS_DEBUG} ${LIBS} ${INCLUDE} main.cpp	

clean:
	rm a.out
