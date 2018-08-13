INCLUDES=	-I/usr/local/opencv/include

CC=	g++

CFLAGS=	-O0	-g	-std=c++11	

CPPLIBS= -L/usr/lib -lopencv_core -lopencv_flann -lopencv_video -lpthread -lrt 
 
CPPFILES= version17.cpp

CPPOBJS= ${CPPFILES:.cpp=.o}

build:	version17

clean:
	-rm -f *.o *.d *.jpg *.ppm
	-rm -f version17

version17:	version17.o	
	$(CC)	$(LDFLAGS)	$(INCLUDES)	$(CFLAGS)	-o	version17	version17.o	`pkg-config --libs opencv`	$(CPPLIBS)
