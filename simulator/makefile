CXX=g++
RM=rm -f
CPPFLAGS= -g -std=c++11 

SRCS=main.cpp
OBJS=$(subst .cc,.o,$(SRCS))

all: main

main: main.cpp
	$(CXX) $(SRCS) -o main

clean:
	$(RM) *.o

