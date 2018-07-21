/*
 *
 *  Example by Sam Siewert 
 *
 */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;
using namespace std;

/*Main function for capturing*/
int main( int argc, char** argv )
{
    cvNamedWindow("Capture Example", CV_WINDOW_AUTOSIZE);
    CvCapture* capture = cvCreateCameraCapture(0); //To capture image
    IplImage* frame;

    while(1)
    {
        frame=cvQueryFrame(capture);  //Frame capture
     
        if(!frame) break;

        cvShowImage("Capture Example", frame);  //OpenCV function to show image

        char c = cvWaitKey(33);
        if( c == 27 ) break;
    }

    cvReleaseCapture(&capture);      //Releasing the capture
    cvDestroyWindow("Capture Example");  //OpenCV function for destroying window
    
};
