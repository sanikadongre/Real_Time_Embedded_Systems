CC= gcc

CFLAGS= -O0 -g -DLINUX -lpthread -lrt -lm -pthread

CFILES1= pthread3.c
CFILES2= pthread3ok.c
CFILES3= deadlock.c
CFILES4= deadlock_timeout.c

SRCS1= ${CFILES1}
SRCS2= ${CFILES2}
SRCS3= ${CFILES3}
SRCS4= ${CFILES4}

OBJS1= ${CFILES1:.c=.o}
OBJS2= ${CFILES2:.c=.o}
OBJS3= ${CFILES3:.c=.o}
OBJS4= ${CFILES4:.c=.o}

build: pthread3 pthread3ok deadlock deadlock_timeout

clean:  -rm -f *.o *.d *.exe pthread3 pthread3ok deadlock deadlock_timeout

pthread3: pthread3.o
        $(CC) $(CFLAGS) -o pthread3 $(OBJS1)

pthread3ok: pthread3ok.o
        $(CC) $(CFLAGS) -o pthread3ok $(OBJS2)

deadlock: deadlock.o
        $(CC) $(CFLAGS) -o deadlock $(OBJS3)

deadlock_timeout: deadlock_timeout.o
        $(CC) $(CFLAGS) -o deadlock_timeout $(OBJS4) 


