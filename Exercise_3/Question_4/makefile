
CC=	gcc

CFLAGS=	-O0	-g	-lpthread	-pthread	-lm


CFILES=	posix_mq.c	heap_mq.c

SRCS=	${CFILES}

OBJS=	${CFILES:.c=.o}

build:	posix_mq	heap_mq


posix_mq:	posix_mq.o
	$(CC)	 $(CFLAGS)	-o	posix_mq	posix_mq.c	-lrt


heap_mq:	heap_mq.o
	$(CC)	 $(CFLAGS)	 -o	 heap_mq	 heap_mq.c	 -lrt


clean:	
	rm	-f	*.o	*.s	*.i	*.d	*.elf	heap_mq	posix_mq
