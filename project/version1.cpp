
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
#define frames_count 2000
#define OK (0)




/*Define semaphores*/
sem_t semaphore, jpeg_semaphore, semaphore_storing, semaphore_fps;

/*struct*/
  struct timespec deadline_frame = {0,20000000};
  struct timespec deadline_jpeg = {0,40000000};
  struct timespec deadline_write = {0, 30000000};



  double framerate;
  double value;

/* Threads for each transform */
pthread_t frame_thread;
pthread_t thread_write;
pthread_t thread_jpeg;
pthread_t thread_fps;

/*pthread attribute structures*/

pthread_attr_t attr_frame;
pthread_attr_t attr_write;
pthread_attr_t attr_jpeg;
pthread_attr_t attr_fps;
pthread_attr_t attr_main_sched;

int max_priority, min_priority; //max and min priority
double initial_time;
/*Defining sched parameters*/

struct sched_param frame_param;
struct sched_param write_param;
struct sched_param jpeg_param;
struct sched_param main_param;
struct sched_param fps_param;
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

double calc_ms(void)
{
  struct timespec scene = {0,0};
  clock_gettime(CLOCK_REALTIME, &scene);
  return (((scene.tv_sec)*1000.0)+((scene.tv_nsec)/MSEC));
}

void *frame_function(void *threadid)
{
  long val;
  int f= 0;
  double end_time = 0.0, run_time = 0.0,old_time = 0.0, jitter_acc = 0.0, jitter_avg = 0.0, jitter_calc=0.0;
  Mat frame;
  while(f< frames_count)
  {
    /*Hold semaphore*/
    sem_wait(&semaphore);
    initial_time = calc_ms();
    printf("frame: %d\n",f);
    printf("start time in ms is: %0.8lf ms \n", initial_time);    
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
    f++;
    end_time = calc_ms();
    printf("\n\r stop time in ms is: %0.8lf ms\n", end_time);
    run_time = end_time - initial_time;
    if(f>0)
    {
      jitter_calc = run_time - old_time;
     }
     jitter_acc += jitter_calc; 
    /*release semaphore for next thread*/
    sem_post(&semaphore_storing);
  }
  jitter_avg = jitter_acc/frames_count;
  printf("The average jitter is: %0.8lf ms\n", jitter_avg);
  pthread_exit(NULL);
}

/* Thread to perform hough transform*/
void *write_function(void *threadid)
{
  long val;
   int f= 0;
  double end_time = 0.0, run_time = 0.0,old_time = 0.0, jitter_acc = 0.0, jitter_avg = 0.0, jitter_calc=0.0;
   Mat gray, canny_frame, cdst;
    vector<Vec4i> lines;  
  while(f< frames_count)
  {
    /*Hold semaphore*/
    sem_wait(&semaphore_storing);
    initial_time = calc_ms();
    printf("frame: %d\n",f);
    printf("start time in ms is: %0.8lf ms \n", initial_time);  
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
     f++;
    end_time = calc_ms();
    printf("\n\r stop time in ms is: %0.8lf ms\n", end_time);
    run_time = end_time - initial_time;
    if(f>0)
    {
      jitter_calc = run_time - old_time;
     }
     jitter_acc += jitter_calc;  
    /*Release semaphore for next thread*/
    sem_post(&jpeg_semaphore);
  }
   jitter_avg = jitter_acc/frames_count;
   printf("The average jitter is: %0.8lf ms\n", jitter_avg);
    pthread_exit(NULL);
}

/* Thread to perform hough eliptical transform*/
void *jpeg_function(void *threadid)
{
  long val;
  Mat gray;
    vector<Vec3f> circles;
    int f = 0;
   double end_time = 0.0, run_time = 0.0,old_time = 0.0, jitter_acc = 0.0, jitter_avg = 0.0, jitter_calc=0.0;
  while(f< frames_count)
  {
    /*Hold semaphhore*/
    sem_wait(&jpeg_semaphore);
    initial_time = calc_ms();
    printf("frame: %d\n",f);
    printf("start time in ms is: %0.8lf ms \n", initial_time);  
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
    printf("circles.size = %d\n", circles.size());
    f++;
    end_time = calc_ms();
    printf("\n\r stop time in ms is: %0.8lf ms\n", end_time);
    run_time = end_time - initial_time;
    if(f>0)
    {
      jitter_calc = run_time - old_time;
    }
     jitter_acc += jitter_calc; 
    /*Release semaphore for next thread*/
    sem_post(&semaphore_fps); 
  }
  jitter_avg = jitter_acc/frames_count;
  printf("The average jitter is: %0.8lf ms\n", jitter_avg);
  pthread_exit(NULL);
}
 
void *fps_function(void *threadid)
{
  sem_wait(&semaphore_fps);
  printf("framepersecondthread\n");
  sem_post(&semaphore);
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
        
        pthread_attr_init(&attr_fps);
	pthread_attr_setinheritsched(&attr_fps,PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setschedpolicy(&attr_fps,SCHED_FIFO);
	fps_param.sched_priority=max_priority-4;

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

        if (sem_init (&semaphore_fps, 0, 0))
	{
	printf ("Failed to initialize semaphore_fps semaphore\n");
	exit (-1);
	}

	pthread_attr_setschedparam(&attr_frame, &frame_param);
	pthread_attr_setschedparam(&attr_write, &write_param);
	pthread_attr_setschedparam(&attr_jpeg, &jpeg_param);
        pthread_attr_setschedparam(&attr_fps, &fps_param);
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
       if(pthread_create(&thread_fps, &attr_fps, fps_function , (void *)0) !=0){
		perror("ERROR; pthread_create:");
		exit(-1);
	}

  printf("\n\rDone creating threads\r\n");
  printf("\n\n\n\rHorizontal resolution:%d and Vertical resolution:%d\n\r",HRES,VRES);
  /* Wait for threads to exit */
  pthread_join(frame_thread,NULL);
  pthread_join(thread_write,NULL);
  pthread_join(thread_jpeg,NULL);
  pthread_join(thread_fps,NULL);

  cvReleaseCapture(&capture);
  cvDestroyWindow("Capture Example");

  var=sched_setscheduler(getpid(), SCHED_OTHER, &nrt_param);

  printf("All done\n");
}
