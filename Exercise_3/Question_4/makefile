
CC=gcc

CFLAGS= -Wall \
-O0



posix_mq:posix_mq.c
	$(CC) $(CFLAGS)  -o posix_mq   posix_mq.c -lpthread -lrt


heap_mq:heap_mq.c
	$(CC) $(CFLAGS)  -o heap_mq   heap_mq.c -lpthread -lrt

.PHONY:clean
clean:
	rm heap_mq posix_mq
