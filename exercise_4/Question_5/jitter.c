/****************************************************************************************************
* Author: Sanika Dongre
* Date: 07/20/18
* Reference:Sam Siewart: http://mercury.pr.erau.edu/~siewerts/cs415/code/computer_vision_cv3_tested/       
****************************************************************************************************/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <time.h>
#include <sys/param.h>
#include <pthread.h>
#include <semaphore.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp> 
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;

struct timespec deadline_canny = {0,20000000};
struct timespec deadline_hough = {0,30000000};
struct timespec deadline_hough_eliptical= {0,40000000};

#define HRES 160
#define VRES 120
#define MSEC 1000000
#define NSEC_PER_SEC (1000000000)
#define OK (0)




/*Define semaphores*/
sem_t semaphore_canny, semaphore_hough, semaphore_hough_eliptical;


      struct timespec initialsec;
      struct timespec endsec;
      struct timespec deltatime;
      struct timespec jitter;


       double framerate;
       double value;

/* Threads for each transform */
pthread_t thread_canny;
pthread_t thread_hough_eliptical;
pthread_t thread_hough;

/*pthread attribute structures*/
pthread_attr_t attr_canny;
pthread_attr_t attr_hough;
pthread_attr_t attr_hough_eliptical;
pthread_attr_t attr_main_sched;

int max_priority, min_priority;



struct sched_param canny_param;
struct sched_param hough_elip_param;
struct sched_param hough_param;
struct sched_param main_param;
struct sched_param nrt_param;


// Transform display window
char timg_window_name[] = "Edge Detector Transform";

/* Canny transform parameters */
int lowThreshold = 0;
int const max_lowThreshold = 100;
int kernel_size = 3;
int edgeThresh = 1;
int ratio = 3;
Mat canny_frame,cdst, timg_gray, timg_grad;

/*Define variable to store each frame as image*/
IplImage* frameCanny;
IplImage* frameHough;
IplImage* frameHoughElip;

CvCapture* capture;


int delta_t(struct timespec *stop, struct timespec *start, struct timespec *delta_t)
{
  int dt_sec=stop->tv_sec - start->tv_sec;
  int dt_nsec=stop->tv_nsec - start->tv_nsec;

  if(dt_sec >= 0)
  {
    if(dt_nsec >= 0)
    {
      delta_t->tv_sec=dt_sec;
      delta_t->tv_nsec=dt_nsec;
    }
    else
    {
      delta_t->tv_sec=dt_sec-1;
      delta_t->tv_nsec=NSEC_PER_SEC+dt_nsec;
    }
  }
  else
  {
    if(dt_nsec >= 0)
    {
      delta_t->tv_sec=dt_sec;
      delta_t->tv_nsec=dt_nsec;
    }
    else
    {
      delta_t->tv_sec=dt_sec-1;
      delta_t->tv_nsec=NSEC_PER_SEC+dt_nsec;
    }
  }

  return(OK);
}


/* Function to perform Canny transform based on threshold */
void CannyThreshold(int, void*)
{
    Mat mat_frame(frameCanny);

    cvtColor(mat_frame, timg_gray, CV_RGB2GRAY);

    // Reduce noise with a kernel 3x3
    blur( timg_gray, canny_frame, Size(3,3) );

    // Canny detector
    Canny( canny_frame, canny_frame, lowThreshold, lowThreshold*ratio, kernel_size );

    // Using Canny's output as a mask, we display our result
    timg_grad = Scalar::all(0);

    mat_frame.copyTo( timg_grad, canny_frame);

    /*Open window to display result*/
    //imshow( "Canny Tranasform", timg_grad );

}

/* Thread to perform canny transform*/
void *canny_function(void *threadid)
{
  long val = 0;
  while(1){
    /*Hold semaphore*/
    sem_wait(&semaphore_canny);

    /*Get start time*/
    clock_gettime(CLOCK_REALTIME, &initialsec);
    printf("Resolution is 160x120\n");
    printf("\n\rTimestamp for the canny transform when it starts: Seconds:%ld and Nanoseconds:%ld",initialsec.tv_sec, initialsec.tv_nsec);
    
    /*Capture and store frame*/
    frameCanny=cvQueryFrame(capture);

    if(!frameCanny)
      return(0);

    CannyThreshold(0, 0);

    char q = cvWaitKey(33);
    if( q == 'q' )
    {
        printf("got quit\n");
        return(0);
    }

    clock_gettime(CLOCK_REALTIME, &endsec);
    printf("\n\rTimestamp for the canny transform when the capture is stopped Seconds:%ld and Nanoseconds:%ld",endsec.tv_sec, endsec.tv_nsec);
    delta_t(&endsec, &initialsec, &deltatime);
    printf("\n\rThe time difference between start and stop is Seconds:%ld and Nanoseconds:%ld", deltatime.tv_sec, deltatime.tv_nsec);
    framerate = NSEC_PER_SEC / deltatime.tv_nsec;
    printf("\n\rThe frame rate is %f", framerate); 
    delta_t(&deadline_canny, &deltatime, &jitter);
    printf("\n\rCanny transform when Jitter is as shown in seconds %ld nanoseconds\n\r", jitter);

    /*release semaphore for next thread*/
    sem_post(&semaphore_hough);
  }
  pthread_exit(NULL);
}

/* Thread to perform hough transform*/
void *hough_function(void *threadid)
{
  long val;
  while(1){

    /*Hold semaphore*/
    sem_wait(&semaphore_hough);

    Mat gray, canny_frame, cdst;
    vector<Vec4i> lines;

    /*Get start time*/
    clock_gettime(CLOCK_REALTIME, &initialsec);
    printf("Resolution is 160x120\n");
    printf("\n\rTimestamp obtained when hough tranform starts: Seconds:%ld and Nanoseconds:%ld",initialsec.tv_sec, initialsec.tv_nsec);
    
    /*Capture and store frame*/
    frameHough=cvQueryFrame(capture);

    /*Convert to matrix*/
    Mat mat_frame(frameHough);
    Canny(mat_frame, canny_frame, 50, 200, 3);

    cvtColor(canny_frame, cdst, CV_GRAY2BGR);
    cvtColor(mat_frame, gray, CV_BGR2GRAY);

    HoughLinesP(canny_frame, lines, 1, CV_PI/180, 50, 50, 10);

    for( size_t i = 0; i < lines.size(); i++ )
    {
      Vec4i l = lines[i];
      line(mat_frame, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,0,255), 3, CV_AA);
    }


    if(!frameHough)
      return 0;

    //cvShowImage("Capture Example", frameHough);

    char q = cvWaitKey(33);
    if( q == 'q' )
    {
        printf("got quit\n");
        return(0);
    }
    /*Get capture stop time*/
    clock_gettime(CLOCK_REALTIME, &endsec);
    printf("\n\rTimestamp for hough transform when capture is stopped:%ld and Nanoseconds:%ld",endsec.tv_sec, endsec.tv_nsec);
    delta_t(&endsec, &initialsec, &deltatime);
    printf("\n\rThe time difference between start and stop is Seconds: %ld and Nanoseconds:%ld", deltatime.tv_sec, deltatime.tv_nsec);
    /*Calculate jitter*/
    framerate = NSEC_PER_SEC/deltatime.tv_nsec;
    printf("\n\rThe frame rate is %f", framerate);
    delta_t(&deadline_hough, &deltatime, &jitter);
    printf("\n\rHough Jitter obtained is %ld nanoseconds\n\r", jitter);
    /*Release semaphore foor next thread*/
    sem_post(&semaphore_hough_eliptical);
  }
    pthread_exit(NULL);
}

/* Thread to perform hough eliptical transform*/
void *hough_elip_function(void *threadid)
{
  long val;
  while(1){
    /*Hold semaphhore*/
    sem_wait(&semaphore_hough_eliptical);

    Mat gray;
    vector<Vec3f> circles;

    /*Get capture start time*/
    clock_gettime(CLOCK_REALTIME, &initialsec);
    printf("Resolution is 160x120\n");
    printf("\n\rTimestamp for hough eliptical capture as it starts: Seconds:%ld and Nanoseconds:%ld",initialsec.tv_sec, initialsec.tv_nsec);
    

    /*Capture and store frame*/
    frameHoughElip=cvQueryFrame(capture);

    /*convert image to matrix and then to grayscale*/
    Mat mat_frame(frameHoughElip);
    cvtColor(mat_frame, gray, CV_BGR2GRAY);
    GaussianBlur(gray, gray, Size(9,9), 2, 2);

    HoughCircles(gray, circles, CV_HOUGH_GRADIENT, 1, gray.rows/8, 100, 50, 0, 0);

    for( size_t i = 0; i < circles.size(); i++ )
    {
      Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
      int radius = cvRound(circles[i][2]);
      // circle center
      circle( mat_frame, center, 3, Scalar(0,255,0), -1, 8, 0 );
      // circle outline
      circle( mat_frame, center, radius, Scalar(0,0,255), 3, 8, 0 );
    }


    if(!frameHoughElip)
      return(0);

    ///cvShowImage("Capture Example", frameHoughElip);

    char q = cvWaitKey(33);
    if( q == 'q' )
    {
        printf("got quit\n");
        return(0);
    }

    /*Get capture stop time*/

    clock_gettime(CLOCK_REALTIME, &endsec);
    printf("\n\rTimestamp for hough eliptical transform as it ends: Seconds:%ld and Nanoseconds:%ld\n",endsec.tv_sec, endsec.tv_nsec);
    delta_t(&endsec, &initialsec, &deltatime);

    printf("circles.size = %d\n", circles.size());
    printf("\n\rThe time difference between start and stop is Seconds: %ld and Nanoseconds:%ld", deltatime.tv_sec, deltatime.tv_nsec);
    framerate = NSEC_PER_SEC/ deltatime.tv_nsec;
    printf("\n\rThe frame rate is %f\n", framerate); 
    /*Calculate jitter*/	  
    delta_t(&deadline_hough_eliptical, &deltatime, &jitter);
    printf("Hough eliptical Jitter obtained is %ld ms\n\r", jitter);

    /*Release semaphore for next thread*/
    sem_post(&semaphore_canny);
  }
  pthread_exit(NULL);
}

/* Print the current scheduling policy */
void print_scheduler(void)
{
   int schedType;

   schedType = sched_getscheduler(getpid());

   switch(schedType)
   {
     case SCHED_FIFO:
	   printf("Pthread Policy is SCHED_FIFO\n");
	   break;
     case SCHED_OTHER:
	   printf("Pthread Policy is SCHED_OTHER\n");
       break;
     case SCHED_RR:
	   printf("Pthread Policy is SCHED_RR\n");
	   break;
     default:
       printf("Pthread Policy is UNKNOWN\n");
   }
}

int main(int argc, char** argv)
{
	int dev=0;
	int var, scope;

	/*Define gui window to open*/
	//cvNamedWindow( timg_window_name, CV_WINDOW_NORMAL);

	// Create a Trackbar for user to enter threshold
	//createTrackbar( "Min Threshold:", timg_window_name, &lowThreshold, max_lowThreshold, CannyThreshold );
    
	/* Select the camera device */
	capture = (CvCapture *)cvCreateCameraCapture(dev);
	
	/* Set the resolution */
	cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH, HRES);
	cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, VRES);

	printf("Before adjustments to scheduling policy:\n");
	print_scheduler();

	max_priority = sched_get_priority_max(SCHED_FIFO);
	min_priority = sched_get_priority_min(SCHED_FIFO);

	/*Initialise threads and attributes*/
	pthread_attr_init(&attr_main_sched);
	pthread_attr_setinheritsched(&attr_main_sched,PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setschedpolicy(&attr_main_sched,SCHED_FIFO);
	main_param.sched_priority=max_priority;

	pthread_attr_init(&attr_canny);
	pthread_attr_setinheritsched(&attr_canny,PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setschedpolicy(&attr_canny,SCHED_FIFO);
	canny_param.sched_priority=max_priority-1;

	pthread_attr_init(&attr_hough_eliptical);
	pthread_attr_setinheritsched(&attr_hough,PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setschedpolicy(&attr_hough,SCHED_FIFO);
	hough_param.sched_priority=max_priority-2;

	pthread_attr_init(&attr_hough);
	pthread_attr_setinheritsched(&attr_hough_eliptical,PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setschedpolicy(&attr_hough_eliptical,SCHED_FIFO);
	hough_elip_param.sched_priority=max_priority-3;

	/* Set scheduling policy */
	var=sched_getparam(getpid(), &nrt_param);
	if (var)
	{
		printf("ERROR; sched_setscheduler var is %d\n", var);
		perror(NULL);
		exit(-1);
	}

	var=sched_setscheduler(getpid(),SCHED_FIFO,&main_param);
	if(var)
	{
		printf("ERROR; main sched_setscheduler var is %d\n",var);
		perror(NULL);
		exit(-1);
	}

	printf("After adjustments to scheduling policy:\n");
	print_scheduler();

	pthread_attr_getscope(&attr_canny, &scope);

	if(scope == PTHREAD_SCOPE_SYSTEM)
	  printf("PTHREAD SCOPE SYSTEM\n");
	else if (scope == PTHREAD_SCOPE_PROCESS)
	  printf("PTHREAD SCOPE PROCESS\n");
	else
	  printf("PTHREAD SCOPE UNKNOWN\n");


	// initialize the signalling semaphores
	if (sem_init (&semaphore_canny, 0, 1))
	{
	printf ("Failed to initialize semaphore_canny semaphore\n");
	exit (-1);
	}

	if (sem_init (&semaphore_hough, 0, 0))
	{
	printf ("Failed to initialize semaphore_hough semaphore\n");
	exit (-1);
	}

	if (sem_init (&semaphore_hough_eliptical, 0, 0))
	{
	printf ("Failed to initialize semaphore_hough_eliptical semaphore\n");
	exit (-1);
	}

	pthread_attr_setschedparam(&attr_canny, &canny_param);
	pthread_attr_setschedparam(&attr_hough, &hough_param);
	pthread_attr_setschedparam(&attr_hough_eliptical, &hough_elip_param);
	pthread_attr_setschedparam(&attr_main_sched, &main_param);

	printf("\n\rCreating threads\r\n");
	
	/* Create threads */
	if(pthread_create(&thread_canny, &attr_canny, canny_function, (void *)0) !=0){
		perror("ERROR; pthread_create:");
		exit(-1);
	}

	if(pthread_create(&thread_hough, &attr_hough, hough_function, (void *)0) !=0){
		perror("ERROR; pthread_create:");
		exit(-1);
	}
	
       if(pthread_create(&thread_hough_eliptical, &attr_hough_eliptical, hough_elip_function , (void *)0) !=0){
		perror("ERROR; pthread_create:");
		exit(-1);
	}

  printf("\n\rDone creating threads\r\n");
  printf("\n\n\n\rHorizontal resolution:%d and Vertical resolution:%d\n\r",HRES,VRES);
  /* Wait for threads to exit */
  pthread_join(thread_canny,NULL);
  pthread_join(thread_hough,NULL);
  pthread_join(thread_hough_eliptical,NULL);

  cvReleaseCapture(&capture);
  cvDestroyWindow("Capture Example");

  var=sched_setscheduler(getpid(), SCHED_OTHER, &nrt_param);

  printf("All done\n");
}
