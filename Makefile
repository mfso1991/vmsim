
CC = g++
CFLAGS = -O2 -std=gnu++0x

vmsim : vmsim.cpp pr.cpp pr.hpp
	${CC} ${CFLAGS} vmsim.cpp pr.cpp pr.hpp -o vmsim
