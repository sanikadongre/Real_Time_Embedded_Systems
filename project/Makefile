INCLUDES=	-I/usr/local/opencv/include

CC=	g++

CFLAGS=	-O0	-g	-std=c++11	

CPPLIBS= -L/usr/lib -lopencv_core -lopencv_flann -lopencv_video -lpthread -lrt 
 
CPPFILES= version4.cpp

CPPOBJS= ${CPPFILES:.cpp=.o}

build:	version4

clean:
	-rm -f *.o *.d *.jpg *.ppm
	-rm -f version3

version4:	version4.o	
	$(CC)	$(LDFLAGS)	$(INCLUDES)	$(CFLAGS)	-o	version4	version4.o	`pkg-config --libs opencv`	$(CPPLIBS)		
