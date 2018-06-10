
CC=gcc

CFLAGS= -O0 -g -lpthread -pthread -lm 

CFILES= Q4_code.c


SRCS= ${CFILES}

OBJS= ${CFILES:.c=.o}

build:	 Q4_code

clean:	 -rm -f Q4_code

Q4_code:	 Q4_code.o
	$(CC) $(CFLAGS) -o Q4_code Q4_code.o 

