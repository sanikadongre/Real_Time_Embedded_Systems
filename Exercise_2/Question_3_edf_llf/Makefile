 
CC=gcc

CFLAGS= -O0 -g -lm
 
CFILES= feasibility_tests.c

SRCS= ${CFILES}

OBJS= ${CFILES:.c=.o}

build:	feasibility_tests

clean:	-rm -f feasibility_tests_code

feasibility_tests:	 feasibility_tests.o
	$(CC) $(CFLAGS) -o feasibility_tests feasibility_tests.o 

