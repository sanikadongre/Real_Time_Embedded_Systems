
CFLAGS= -O -g $(INCLUDE_DIRS) $(CDEFS) -DLINUX

LIBS=-lpthread -lrt
 
CFILES2= deadlock_fix.c

SRCS2= ${HFILES} ${CFILES2}

OBJS2= ${CFILES2:.c=.o}

all: deadlock_fix

clean:
	-rm -f *.o *.d *.exe deadlock_fix

deadlock_fix: deadlock_fix.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $(OBJS2) $(LIBS)

depend:

.c.o:
	$(CC) -MD $(CFLAGS) -c $<
