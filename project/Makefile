INCLUDES=	-I/usr/local/opencv/include

CC=	g++

CFLAGS=	-O0	-g	-std=c++11	

CPPLIBS= -L/usr/lib -lopencv_core -lopencv_flann -lopencv_video -lpthread -lrt 
 
CPPFILES= version16.cpp

CPPOBJS= ${CPPFILES:.cpp=.o}

build:	version16

clean:
	-rm -f *.o *.d *.jpg *.ppm
	-rm -f version16

version16:	version16.o	
	$(CC)	$(LDFLAGS)	$(INCLUDES)	$(CFLAGS)	-o	version16	version16.o	`pkg-config --libs opencv`	$(CPPLIBS)
