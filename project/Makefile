
CC=	g++

CFLAGS=	-O0	-g	-std=c++11	

CPPLIBS= -L/usr/lib -lopencv_core -lopencv_flann -lopencv_video -lpthread -lrt 
 
CPPFILES= capture.cpp

CPPOBJS= ${CPPFILES:.cpp=.o}

build:	capture 

clean:
	-rm -f *.o *.d
	-rm -f capture

capture:	capture.o
	$(CC)	$(LDFLAGS)	$(CFLAGS)	-o	capture	capture.o	`pkg-config --libs opencv`	$(CPPLIBS)	
