#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <time.h>
#include <sys/param.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <mqueue.h>
#include <stdbool.h>
#include <fstream>
#include <sstream>
#include <iomanip>



using namespace cv;
using namespace std;

VideoCapture cap(0);

Mat ppm_frame(480,640,CV_8UC3);
uint8_t *frame_ptr;
Mat frame_jpg(480,640,CV_8UC3);
#define HRES 640
#define VRES 480
#define MSEC 1000000
#define NSEC_PER_SEC (1000000000)
#define frames_count  1800
#define threads_count 3
#define OK (0)
#define TRUE (1)
#define FALSE (0)
#define ERROR (-1)


int abortTest = FALSE;

double framerate;
double value;
int cap_count= 0;
double diff=0;
int device = 0;
time_t rawtime;
struct tm * timecur;
sem_t semaphore_arr[threads_count];
pthread_t thread_arr[threads_count];
pthread_attr_t attr_arr[threads_count];
struct sched_param param_arr[threads_count];
void* (*func_arr[threads_count]) (void*) ;
double start_arr[threads_count] = {0,0,0};
double stop_arr[threads_count] = {0,0,0};
double acc_jitter_arr[threads_count] = {0,0,0};
double avg_jitter_arr[threads_count] = {0,0,0};
double wcet_arr[threads_count] = {0,0,0};
double avg_diff_arr[threads_count] = {0,0,0};
uint32_t counter_arr[threads_count] = {0,0,0};
double jitter_calc_arr[threads_count] = {0,0,0};
double run_time[threads_count] = {0,0,0};
static struct timespec start_time, stop_time, exe_time, current_time;
static uint32_t timer_counter = 0, start_sec=0;
static uint8_t cond = TRUE;
static struct timespec cap_start_time = {0,0};
static struct timespec cap_stop_time = {0,0};
static struct mq_attr frame_mq_attr;
double initial_time;
sem_t ppm_sem, jpg_sem, jpg_done_sem, ppm_done_sem, camera_sem, ts_sem, ts1_sem;

void delta_t(struct timespec *stop, struct timespec *start, struct timespec *delta_t)
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
  return;
}

void threads_init(void)
{
	uint8_t i=0; //max_priority=20;
	uint8_t max_priority = sched_get_priority_max(SCHED_FIFO);
	for(i=0;i<threads_count;i++)
	{
		pthread_attr_init(&attr_arr[i]);
		pthread_attr_setinheritsched(&attr_arr[i],PTHREAD_EXPLICIT_SCHED);
		pthread_attr_setschedpolicy(&attr_arr[i],SCHED_FIFO);
		param_arr[i].sched_priority=max_priority-i;		
		sem_init (&semaphore_arr[i], 0, 0);
		sem_init(&ppm_sem, 0,1);
	        sem_init(&jpg_sem, 0,1);
		sem_init(&ppm_done_sem, 0,0);
		sem_init(&jpg_done_sem, 0,0);
		sem_init(&camera_sem, 0,1);
        	pthread_attr_setschedparam(&attr_arr[i], &param_arr[i]);
		if(pthread_create(&thread_arr[i], &attr_arr[i], func_arr[i], (void *)0) !=0)
		{
			cout<<"\nthread "<<i+0<<" created";
			}
	}
	
	sem_post(&semaphore_arr[1]);
	for(i=0;i<threads_count;i++)
	{
  		pthread_join(thread_arr[i],NULL);
	}
}

void time_check(void)
{
	if(++timer_counter==frames_count)
	{
		cond = FALSE;
	}
}



void *sequencer(void *threadid)
{
	uint8_t thread_id = 0,i=0;	
	struct timespec delay_time = {0, 25000000};
	struct timespec remaining_time;
	uint32_t time_old = 1;
	unsigned long long seqCnt =0; 
	double residual; int rc; int delay_cnt =0;
	clock_gettime(CLOCK_REALTIME, &current_time);
	do 
	{       
		//sem_wait(&semaphore_arr[0]);
		clock_gettime(CLOCK_REALTIME, &current_time);
		delay_cnt=0; residual=0.0;
		{
			start_sec++;
			time_old = stop_time.tv_sec;
			do
			{
				rc=nanosleep(&delay_time, &remaining_time);
				if(rc==EINTR)
				{
					residual = remaining_time.tv_sec + ((double)remaining_time.tv_nsec/(double) NSEC_PER_SEC);
					delay_cnt++;
				}
				else if(rc < 0)
				{
					perror(" Sequencer nanosleep");
					exit(-1);
				}
			}while((residual > 0.0) && (delay_cnt < 100));
			seqCnt++;
			clock_gettime(CLOCK_REALTIME, &current_time);
				if(delay_cnt > 1)
			if((seqCnt % 4) == 0)
			{
				time_check();
				sem_post(&semaphore_arr[1]);
				//sem_wait(&semaphore_arr[0]);
			}
			if((seqCnt % 4) == 0)
			{
				
				sem_post(&semaphore_arr[2]);
				//sem_wait(&semaphore_arr[0]);
			}
		}
		sem_post(&semaphore_arr[0]);

	}while(cond);
	for(i=1;i<threads_count;i++)
	{
		sem_post(&semaphore_arr[i]);
		//sem_wait(&semaphore_arr[0]);
	}
	pthread_exit(NULL);
}

void *frame_function(void *threadid)
{
	
  	uint8_t thread_id=1;	
	
	system("uname -a > system.out");
 	while(cond)
  	{
    		/*Hold semaphore*/
    		sem_wait(&semaphore_arr[thread_id]);
		sem_wait(&ppm_sem);
		cap >> ppm_frame;
		sem_post(&ppm_sem);
		frame_ptr = (uint8_t*) ppm_frame.data;
		sem_post(&semaphore_arr[0]);
	}
	//sem_post(&semaphore_arr[0]);
	pthread_exit(NULL);
}

void *write_function(void *threadid)
{
	
	uint8_t thread_id=2;
	ostringstream name;
	vector<int> compression_params;
	compression_params.push_back(CV_IMWRITE_PXM_BINARY);
	compression_params.push_back(95);
 	while(cond)
	{
    		/*Hold semaphore*/
		sem_wait(&ppm_sem);
    		sem_wait(&semaphore_arr[thread_id]);
		name.str("frame_");
		name<<"frame_"<<counter_arr[thread_id]<<".ppm";
		time (&rawtime);
 		timecur = localtime (&rawtime);
		putText(ppm_frame,asctime(timecur),Point(465,470),FONT_HERSHEY_COMPLEX_SMALL,0.7,Scalar(255,255,0),2);
		imwrite(name.str(), ppm_frame, compression_params);
		name.str(" ");
		sem_post(&semaphore_arr[0]);
		sem_post(&ppm_done_sem);
		sem_post(&ts1_sem);
		sem_post(&ppm_sem);
		
  	}
	pthread_exit(NULL);
}

/*The main function*/
int main(int argc, char *argv[])
{
	clock_gettime(CLOCK_REALTIME, &start_time);
	//printf("\nThe start time is %d seconds and %d nanoseconds\n", start_time.tv_sec, start_time.tv_nsec);

	if(argc > 1)
	{
		sscanf(argv[1], "%d", &device);
	}
	cap.set(CV_CAP_PROP_FRAME_WIDTH, HRES);
	cap.set(CV_CAP_PROP_FRAME_HEIGHT, VRES);
	cap.open(device);
	func_arr[0] = sequencer;
  	func_arr[1] = frame_function;
  	func_arr[2] = write_function;
 	threads_init();
	cap.release();
	clock_gettime(CLOCK_REALTIME, &stop_time);
	printf("\nThe code stop time is %d seconds and %d nanoseconds\n",stop_time.tv_sec, stop_time.tv_nsec);
	delta_t(&stop_time, &start_time, &exe_time);
	cout<<"\n\r The execution time for the code is: "<<exe_time.tv_sec<<" seconds "<<exe_time.tv_nsec<<" nano seconds.\n";
	printf("\nAll done\n");
}

	
