
CC=	gcc

CFLAGS=	-O0	-g

LIBS=	-lrt
 
CFILES= capture.c

SRCS=	${CFILES}
OBJS=	${CFILES:.c=.o}

build:	capture

clean:
	-rm -f *.o *.d
	-rm -f capture

capture:	${OBJS}
	$(CC)	$(LDFLAGS)	$(CFLAGS)	-o	capture	capture.o	$(LIBS)

