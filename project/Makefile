INCLUDES=	-I/usr/local/opencv/include

CC=	g++

CFLAGS=	-O0	-g	-std=c++11	

CPPLIBS= -L/usr/lib -lopencv_core -lopencv_flann -lopencv_video -lpthread -lrt 
 
CPPFILES= version7.cpp

CPPOBJS= ${CPPFILES:.cpp=.o}

build:	version7

clean:
	-rm -f *.o *.d *.jpg *.ppm
	-rm -f version6

version7:	version7.o	
	$(CC)	$(LDFLAGS)	$(INCLUDES)	$(CFLAGS)	-o	version7	version7.o	`pkg-config --libs opencv`	$(CPPLIBS)		
