CC = gcc

CFLAGS= -O0 -lpthread -lrt
 
DEADLOCK_FIX_EXE= deadlock_fix

CFILES= deadlock_fix.c

SRCS= ${CFILES}

OBJS= ${CFILES:.c=.o}

build:	${DEADLOCK_FIX_EXE}

clean:	-RM -F ${DEADLOCK_FIX_EXE}

deadlock_fix: deadlock_fix.o
	$(CC) $(CFLAGS) -o deadlock_fix deadlock_fix.o


