 
CC=gcc
 
CFLAGS= -O3 -lpthread -lrt

POSIX_EXE= posix_clock_code

CFILES= posix_clock.c

SRCS= ${CFILES}

OBJS= ${CFILES:.c=.o}

build:	${POSIX_EXE}

clean:	-rm -f ${POSIX_EXE} 

posix_clock_code:	posix_clock.o
	$(CC) $(CFLAGS) -o posix_clock_code posix_clock.o 


