INCLUDE_DIRS = 
LIB_DIRS = 
CC=gcc

CDEFS=
CFLAGS= -O0 -g $(INCLUDE_DIRS) $(CDEFS)
LIBS= 

HFILES= 
CFILES= Q4_code.c


SRCS= ${HFILES} ${CFILES}
OBJS= ${CFILES:.c=.o}

all:	 Q4_code

clean:	 -rm -f Q4_code

Q4_c:	 Q4_code.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $@.o -lpthread

depend:

.c.o:	$(CC) $(CFLAGS) -c $<
