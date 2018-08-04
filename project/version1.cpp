
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

struct timespec time_end = {0,0};
struct timespec time_left = {0,0};


#define HRES 640
#define VRES 480
#define MSEC 1000000
#define NSEC_PER_SEC (1000000000)
#define OK (0)




/*Define semaphores*/
sem_t semaphore, jpeg_semaphore, semaphore_storing;

/*struct*/
  struct timespec deadline_frame = {0,20000000};
  struct timespec deadline_jpeg = {0,40000000};
  struct timespec deadline_write = {0, 30000000};
  struct timespec initialsec;
  struct timespec endsec;
  struct timespec deltatime;
  struct timespec jitter;


  double framerate;
  double value;

/* Threads for each transform */
pthread_t frame_thread;
pthread_t thread_write;
pthread_t thread_jpeg;

/*pthread attribute structures*/
pthread_attr_t attr_frame;
pthread_attr_t attr_write;
pthread_attr_t attr_jpeg;
pthread_attr_t attr_main_sched;

int max_priority, min_priority; //max and min priority

/*Defining sched parameters*/

struct sched_param frame_param;
struct sched_param write_param;
struct sched_param jpeg_param;
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

/*Function to calculate difference between stop and start time*/
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

void *frame_function(void *threadid)
{
  long val;
  int f= 0;
  double end_time = 0, run_time = 0,old_time = 0, jitter_acc = 0, jitter_avg = 0;
  Mat frame;
  while(1){
    /*Hold semaphore*/
    sem_wait(&semaphore);

    /*Get start time*/
    clock_gettime(CLOCK_REALTIME, &initialsec);
    printf("Resolution is 640x480\n");
    printf("\n\rTimestamp for the canny transform when it starts: Seconds:%ld and Nanoseconds:%ld",initialsec.tv_sec, initialsec.tv_nsec);
    
    /*Capture and store frame*/
    frameCanny=cvQueryFrame(capture);

    if(!frameCanny)
      return(0);

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
    delta_t(&deadline_frame, &deltatime, &jitter);
    printf("\n\rCanny transform when Jitter is as shown is %ld nanoseconds\n\r", jitter.tv_nsec);

    /*release semaphore for next thread*/
    sem_post(&semaphore_storing);
  }
  pthread_exit(NULL);
}

/* Thread to perform hough transform*/
void *write_function(void *threadid)
{
  long val;
  int f = 0;
  while(1){

    /*Hold semaphore*/
    sem_wait(&semaphore_storing);

    Mat gray, canny_frame, cdst;
    vector<Vec4i> lines;

    /*Get start time*/
    clock_gettime(CLOCK_REALTIME, &initialsec);
    printf("Resolution is 160x120\n");
    printf("\n\rTimestamp obtained when hough tranform starts: Seconds:%ld and Nanoseconds:%ld",initialsec.tv_sec, initialsec.tv_nsec);
    
    /*Capturing and storing of frame*/
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
    /*frame rate per seconds calculation*/
    framerate = NSEC_PER_SEC/deltatime.tv_nsec;
    printf("\n\rThe frame rate is %f", framerate);
    delta_t(&deadline_write, &deltatime, &jitter);
     /*calculating jitter*/
    printf("\n\rHough Jitter obtained is %ld nanoseconds\n\r", jitter.tv_nsec);
    /*Release semaphore for next thread*/
    sem_post(&jpeg_semaphore);
  }
    pthread_exit(NULL);
}

/* Thread to perform hough eliptical transform*/
void *jpeg_function(void *threadid)
{
  long val;
  while(1){
    /*Hold semaphhore*/
    sem_wait(&jpeg_semaphore);

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
    delta_t(&endsec, &initialsec, &deltatime);
    printf("\n\rThe time difference between start and stop is Seconds: %ld and Nanoseconds:%ld", deltatime.tv_sec, deltatime.tv_nsec);
    /*frame rate per seconds calculation*/
    framerate = NSEC_PER_SEC/deltatime.tv_nsec;
    printf("\n\rThe frame rate is %f", framerate);                /*Frame rate calculation*/            
    delta_t(&deadline_jpeg, &deltatime, &jitter);        /*Calculating jitter*/
    printf("Hough eliptical Jitter obtained is %ld ms\n\r", jitter.tv_nsec); /*jitter print*/

    /*Release semaphore for next thread*/
    sem_post(&semaphore); 
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
/*The main function*/
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
	pthread_attr_setschedpolicy(&attr_main_sched,SCHED_FIFO);    /*sched_fifo attributes*/
	main_param.sched_priority=max_priority;

	pthread_attr_init(&attr_frame);
	pthread_attr_setinheritsched(&attr_frame,PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setschedpolicy(&attr_frame,SCHED_FIFO);
	frame_param.sched_priority=max_priority-1;

	pthread_attr_init(&attr_write);
	pthread_attr_setinheritsched(&attr_write,PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setschedpolicy(&attr_write,SCHED_FIFO);
	write_param.sched_priority=max_priority-2;

	pthread_attr_init(&attr_jpeg);
	pthread_attr_setinheritsched(&attr_jpeg,PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setschedpolicy(&attr_jpeg,SCHED_FIFO);
	jpeg_param.sched_priority=max_priority-3;
        

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

	pthread_attr_getscope(&attr_frame, &scope);

	if(scope == PTHREAD_SCOPE_SYSTEM)
	  printf("PTHREAD SCOPE SYSTEM\n");
	else if (scope == PTHREAD_SCOPE_PROCESS)
	  printf("PTHREAD SCOPE PROCESS\n");  
	else
	  printf("PTHREAD SCOPE UNKNOWN\n");


	// initialize the signalling semaphores
	if (sem_init (&semaphore, 0, 1))
	{
	printf ("Failed to initialize semaphore semaphore\n");
	exit (-1);
	}

	if (sem_init (&semaphore_storing, 0, 0))
	{
	printf ("Failed to initialize semaphore_storing semaphore\n");
	exit (-1);
	}

	if (sem_init (&jpeg_semaphore, 0, 0))
	{
	printf ("Failed to initialize semaphore_jpeg semaphore\n");
	exit (-1);
	}

	pthread_attr_setschedparam(&attr_frame, &frame_param);
	pthread_attr_setschedparam(&attr_write, &write_param);
	pthread_attr_setschedparam(&attr_jpeg, &jpeg_param);
	pthread_attr_setschedparam(&attr_main_sched, &main_param);

	printf("\n\rCreating threads\r\n");
	
	/* Create threads */
	if(pthread_create(&frame_thread, &attr_frame, frame_function, (void *)0) !=0){
		perror("ERROR; pthread_create:");
		exit(-1);
	}

	if(pthread_create(&thread_write, &attr_write, write_function, (void *)0) !=0){
		perror("ERROR; pthread_create:");
		exit(-1);
	}
	
       if(pthread_create(&thread_jpeg, &attr_jpeg, jpeg_function , (void *)0) !=0){
		perror("ERROR; pthread_create:");
		exit(-1);
	}

  printf("\n\rDone creating threads\r\n");
  printf("\n\n\n\rHorizontal resolution:%d and Vertical resolution:%d\n\r",HRES,VRES);
  /* Wait for threads to exit */
  pthread_join(frame_thread,NULL);
  pthread_join(thread_write,NULL);
  pthread_join(thread_jpeg,NULL);

  cvReleaseCapture(&capture);
  cvDestroyWindow("Capture Example");

  var=sched_setscheduler(getpid(), SCHED_OTHER, &nrt_param);

  printf("All done\n");
}
