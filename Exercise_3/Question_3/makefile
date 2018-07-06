CC = gcc

CFLAGS= -O -g -DLINUX -lpthread -lrt
 
CFILES2= deadlock_fix.c

SRCS2= ${CFILES2}

OBJS2= ${CFILES2:.c=.o}

all: deadlock_fix

clean:	-rm -f *.o *.d *.exe deadlock_fix

deadlock_fix: deadlock_fix.o
	$(CC) $(CFLAGS) -o $@ $(OBJS2)

