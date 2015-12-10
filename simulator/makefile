CXX=/s/gcc-4.8.2/bin/g++
RM=rm -f
CPPFLAGS=-std=c++0x 

SRCS=main.cpp
OBJS=$(subst .cc,.o,$(SRCS))

all: main

main: main.cpp
	$(CXX) $(CPPFLAGS) $(SRCS) -o main

clean:
	$(RM) *.o

