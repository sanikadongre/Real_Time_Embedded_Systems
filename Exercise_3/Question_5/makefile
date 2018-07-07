
CC=	gcc

CFLAGS=	-O0	-lpthread	-lrt   

WATCHDOG_TIMER_EXE=	Watchdog_timer

CFILES=	Watchdog_timer.c

SRCS=	${CFILES}

OBJS=	${CFILES:.c=.o}

build:	${WATCHDOG_TIMER_EXE}

Watchdog_timer:	Watchdog_timer.o
	$(CC)	$(CFLAGS)	-o	Watchdog_timer	Watchdog_timer.o 

clean:	
	rm	-f	*.o	*.i	*.s	*.d	*.elf	${Watchdog_timer}
