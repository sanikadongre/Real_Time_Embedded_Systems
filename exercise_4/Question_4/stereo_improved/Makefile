INCLUDES =  -I/usr/local/opencv/include
 
CC=g++

CFLAGS= -O0 -g $(INCLUDES)

LIBS= -lrt

CPPLIBS= -L/usr/local/opencv/lib -lopencv_core -lopencv_flann -lopencv_video

build:	capture capture_stereo

clean:
	-rm -f *.o *.d
	-rm -f capture
	-rm -f capture_stereo

capture_stereo: capture_stereo.o
	$(CC) $(LDFLAGS) $(CFLAGS) $(INCLUDES) -o capture_stereo capture_stereo.o `pkg-config --libs opencv` $(CPPLIBS)

capture: capture.o
	$(CC) $(LDFLAGS) $(CFLAGS) $(INCLUDES) -o capture capture.o `pkg-config --libs opencv` $(CPPLIBS)


