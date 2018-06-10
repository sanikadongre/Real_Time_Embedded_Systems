 
CC=gcc

CFLAGS= -O0 -g -lpthread -pthread
 
CFILES= pthread.c

SRCS= ${CFILES}

OBJS= ${CFILES:.c=.o}

build:	pthread_elf_code

clean:	-rm -f pthread

pthread_elf_code: pthread.o
	$(CC) $(CFLAGS) -o pthread_elf_code pthread.o

