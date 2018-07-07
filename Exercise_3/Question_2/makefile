
CC= gcc

CFLAGS= -O0 -pthread -lrt   

MUTEX_EXE=  mutex

CFILES= mutex.c

SRCS= ${CFILES}

OBJS= ${CFILES:.c=.o}

build:  ${MUTEX_EXE}

mutex:  mutex.o
	$(CC) $(CFLAGS) -o  mutex mutex.c

clean: 
	 -rm	-f	*.o	*.s	*.i	*.d	*.elf	*.swp	 ${MUTEX_EXE}
