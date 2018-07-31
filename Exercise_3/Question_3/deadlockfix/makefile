CC=	 gcc

CFLAGS=	-O0	-lpthread	-lrt	-pthread
 
DEADLOCK_FIX_EXE=	deadlock_fix

CFILES=	deadlock_fix.c

SRCS=	${CFILES}

OBJS=	${CFILES:.c=.o}

build:	${DEADLOCK_FIX_EXE}

clean:
	rm	-f	*.o	*.i	*.s	*.d	*.exe	 ${DEADLOCK_FIX_EXE}

deadlock_fix:	deadlock_fix.o
	$(CC)	$(CFLAGS)	 -o	deadlock_fix	deadlock_fix.c


