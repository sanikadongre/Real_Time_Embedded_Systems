INCLUDES=	-I/usr/include

CC=	g++

CFLAGS=	-O0	-g	-std=c++11	

CPPLIBS= -L/usr/lib -lopencv_core -lopencv_flann -lopencv_video -lpthread -lrt 
 
CPPFILES= jitter.cpp

CPPOBJS= ${CPPFILES:.cpp=.o}

build:	jitter

clean:
	-rm -f *.o *.d
	-rm -f jitter

jitter:	jitter.o	
	$(CC)	$(LDFLAGS)	$(INCLUDES)	$(CFLAGS)	-o	jitter	jitter.o	`pkg-config --libs opencv`	$(CPPLIBS)		
