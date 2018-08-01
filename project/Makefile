
CC=g++

CFLAGS=	-O0	-g

LIBS= -lrt

CPPLIBS= -L/usr/lib -lopencv_core -lopencv_flann -lopencv_video
 
CPPFILES= capture.cpp

CPPOBJS= ${CPPFILES:.cpp=.o}

build:	capture 

clean:
	-rm -f *.o *.d
	-rm -f capture

capture:	capture.o
	$(CC)	$(LDFLAGS)	$(CFLAGS)	-o	capture	capture.o	`pkg-config --libs opencv`	$(CPPLIBS)


