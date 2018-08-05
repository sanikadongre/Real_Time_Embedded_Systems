INCLUDES=	-I/usr/local/opencv/include

CC=	g++

CFLAGS=	-O0	-g	-std=c++11	

CPPLIBS= -L/usr/lib -lopencv_core -lopencv_flann -lopencv_video -lpthread -lrt 
 
CPPFILES= version6.cpp

CPPOBJS= ${CPPFILES:.cpp=.o}

build:	version6

clean:
	-rm -f *.o *.d *.jpg *.ppm
	-rm -f version6

version6:	version6.o	
	$(CC)	$(LDFLAGS)	$(INCLUDES)	$(CFLAGS)	-o	version6	version6.o	`pkg-config --libs opencv`	$(CPPLIBS)		
