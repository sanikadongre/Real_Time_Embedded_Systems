INCLUDES=	-I/usr/local/opencv/include

CC=	g++

CFLAGS=	-O0	-g	-std=c++11	

CPPLIBS= -L/usr/lib -lopencv_core -lopencv_flann -lopencv_video -lpthread -lrt 
 
CPPFILES= version12.cpp

CPPOBJS= ${CPPFILES:.cpp=.o}

build:	version12

clean:
	-rm -f *.o *.d *.jpg *.ppm
	-rm -f version12

version12:	version12.o	
	$(CC)	$(LDFLAGS)	$(INCLUDES)	$(CFLAGS)	-o	version12	version12.o	`pkg-config --libs opencv`	$(CPPLIBS)		
