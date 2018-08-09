#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <time.h>
#include <sys/param.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include <string.h>
#include <mqueue.h>
#include <stdbool.h>
#include <fstream>
#include <sstream>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <opencv/highgui.h>
#include <opencv2/opencv.hpp>
#include <mqueue.h>

using namespace cv;
using namespace std;

#define ppm_mqueue  "/ppm_mq"
#define jpg_mqueue "/jpg_mq"
#define ERROR -1


#define HRES 640
#define VRES 480
#define MSEC 1000000
#define NSEC_PER_SEC (1000000000)
#define frames_count 60
#define threads_count 8
#define OK (0)
#define TRUE (1)
#define FALSE (0)

int abortTest = FALSE;


int device=0;
VideoCapture cap(1);
pthread_mutex_t frame_cap;

double framerate;
double value;

#define cap_count (60)


sem_t semaphore_arr[threads_count];
pthread_t thread_arr[threads_count];
pthread_attr_t attr_arr[threads_count];
struct sched_param param_arr[threads_count];
void* (*func_arr[threads_count]) (void*) ;
double start_arr[threads_count] = {0,0,0,0,0,0,0,0};
double stop_arr[threads_count] = {0,0,0,0,0,0,0,0};
double acc_jitter_arr[threads_count] = {0,0,0,0,0,0,0,0};
double avg_jitter_arr[threads_count] = {0,0,0,0,0,0,0,0};
double wcet_arr[threads_count] = {0,0,0,0,0,0,0,0};
double avg_diff_arr[threads_count] = {0,0,0,0,0,0,0,0};
uint32_t counter_arr[threads_count] = {0,0,0,0,0,0,0,0};
double jitter_calc_arr[threads_count] = {0,0,0,0,0,0,0,0};
double run_time[threads_count] = {0,0,0,0,0,0,0,0};
static struct timespec start_time, stop_time, exe_time, current_time;
static uint32_t timer_counter = 0, start_sec=0;
static uint8_t cond = TRUE;
unsigned long long sequencePeriods[threads_count] ={900};
pthread_mutex_t frame_cap;

double initial_time;

double calc_ms(void)
{
	struct timespec scene = {0,0};
	clock_gettime(CLOCK_REALTIME, &scene);
	return ((scene.tv_sec*1000)+scene.tv_nsec/MSEC);
}

void jitter_calculations(uint8_t thread_id)
{
	
	cout<<"\n\rThread"<<thread_id+0;
 	printf("\n\rframe: %d\n",counter_arr[thread_id]);
    	printf("\n\rstart time in ms is: %0.8lf ms \n", start_arr[thread_id]); 
    	stop_arr[thread_id] = calc_ms();
    	printf("\n\r stop time in ms is: %0.8lf ms\n", stop_arr[thread_id]);
    	run_time[thread_id] = stop_arr[thread_id] - start_arr[thread_id];
	printf("\n\r The run time is : %0.8lf ms\n", run_time[thread_id]);
	if(run_time[thread_id] > wcet_arr[thread_id])
	{
		wcet_arr[thread_id] = run_time[thread_id];
		cout<<"\n\r The worst execution time for thread"<<thread_id+0<<" ="<<wcet_arr[thread_id];
	}
	if(counter_arr[thread_id] == 0)
	{
      		avg_diff_arr[thread_id] = run_time[thread_id];
		printf(" the avg difference array is: %0.8lf ms\n", avg_diff_arr[thread_id]);
    	}
	else if(counter_arr[thread_id] > 0)
	{
		jitter_calc_arr[thread_id] = run_time[thread_id] - avg_diff_arr[thread_id];
		avg_diff_arr[thread_id] = (avg_diff_arr[thread_id] * (counter_arr[thread_id]-1) + run_time[thread_id])/(counter_arr[thread_id]);
  		printf(" The average difference array is: %0.8lf ms\n ", avg_diff_arr[thread_id]);    		
		printf("\rThe calculated jitter is %0.8lf ms\n", jitter_calc_arr[thread_id]);
		acc_jitter_arr[thread_id] += jitter_calc_arr[thread_id];
	}
	counter_arr[thread_id]++;
}

void jitter_final_print(uint8_t thread_id)
{
	cout<<"\n\rThe accumulated jitter for thread "<<thread_id+0<<" ="<<acc_jitter_arr[thread_id];
	cout<<"\n\r The worst execution time for thread"<<thread_id+0<<" ="<<wcet_arr[thread_id];
  	avg_jitter_arr[thread_id]=acc_jitter_arr[thread_id]/frames_count;
	cout<<"\n\rThe average jitter for thread in ms thread"<<thread_id+0<<" ="<<avg_jitter_arr[thread_id]; 

}	
void threads_init(void)
{
	uint8_t i=0; //max_priority=20;
	uint8_t max_priority = sched_get_priority_max(SCHED_FIFO);
	printf("\nCreating Threads");
	for(i=0;i<threads_count;i++)
	{
		pthread_attr_init(&attr_arr[i]);
		pthread_attr_setinheritsched(&attr_arr[i],PTHREAD_EXPLICIT_SCHED);
		pthread_attr_setschedpolicy(&attr_arr[i],SCHED_FIFO);
		param_arr[0].sched_priority=max_priority;
		param_arr[1].sched_priority=max_priority-1;
		param_arr[2].sched_priority=max_priority-2;
		param_arr[3].sched_priority=max_priority-3;
		param_arr[4].sched_priority=max_priority-2;
		param_arr[5].sched_priority=max_priority-3;
		param_arr[6].sched_priority=max_priority-2;
		param_arr[7].sched_priority=max_priority-4;	
		if (sem_init (&semaphore_arr[i], 0, 0))
		{
			cout<<"\n\rFailed to initialize semaphore for thread"<<i;
			exit (-1);
		}
		cout<<"\nSemaphore "<<i+0<<" initialized"; 
        	pthread_attr_setschedparam(&attr_arr[i], &param_arr[i]);
		if(pthread_create(&thread_arr[i], &attr_arr[i], func_arr[i], (void *)0) !=0)
		{
			perror("ERROR; pthread_create:");
			exit(-1);
		}
		cout<<"\nthread "<<i+0<<" created";
	}
	printf("sanu\n");
	sem_post(&semaphore_arr[1]);
	printf("SANU\n");
	cout<<"\n\rjoining Threads";
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
	struct timespec delay_time = {0, 33333333};
	struct timespec remaining_time;
	uint32_t time_old = 1;
	unsigned long long seqCnt =0; 
	double residual; int rc; int delay_cnt =0;
	clock_gettime(CLOCK_REALTIME, &current_time);
	printf("\n\rThe current time is %d sec and %d nsec\n", current_time.tv_sec, current_time.tv_nsec);
	//printf("The stop time is %d seconds and %d nanoseconds\n", stop_time.tv_sec, stop_time.tv_nsec);
	do 
	{       
		sem_wait(&semaphore_arr[0]);
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
			printf("%d seq_loop\n", delay_cnt);
			if((seqCnt % 10) == 0)
			{
				sem_post(&semaphore_arr[1]);
				sem_wait(&semaphore_arr[0]);
			}
			if((seqCnt % 30) == 0)
			{
				sem_post(&semaphore_arr[2]);
				sem_wait(&semaphore_arr[0]);
			}
		  	if((seqCnt % 60) == 0)
			{	
				time_check();
				sem_post(&semaphore_arr[3]);
				sem_wait(&semaphore_arr[0]);
			}
		  	if((seqCnt % 30) == 0)
			{
				sem_post(&semaphore_arr[4]);
				sem_wait(&semaphore_arr[0]);
			}
			if((seqCnt % 60) == 0)
			{
				sem_post(&semaphore_arr[5]);
				sem_wait(&semaphore_arr[0]);
			}
			if((seqCnt % 30) == 0)
			{
				sem_post(&semaphore_arr[6]);
				sem_wait(&semaphore_arr[0]);
			}
			if((seqCnt % 300) == 0)
			{
				sem_post(&semaphore_arr[7]);
				sem_wait(&semaphore_arr[0]);
			}
		}
		sem_post(&semaphore_arr[0]);

	}while(cond);
	for(i=1;i<threads_count;i++)
	{
		sem_post(&semaphore_arr[i]);
		sem_wait(&semaphore_arr[0]);
	}
	pthread_exit(NULL);
}

void *frame_function(void *threadid)
{	
	
	Mat ppm_frame;
  	uint8_t thread_id=1;	
	int i;		
 	while(cond)
  	{
    		/*Hold semaphore*/
    		sem_wait(&semaphore_arr[thread_id]);
	    	start_arr[thread_id] = calc_ms();
		clock_gettime(CLOCK_REALTIME, &start_time);
		printf("camera stop time is: %d\n", start_time.tv_sec);
		cap.open(device);
		
		//Do code here
		printf("\n2nd thread");
		for(i=0; i< cap_count ; i++)
		{	
			pthread_mutex_lock(&frame_cap);
			cap >> ppm_frame;
			pthread_mutex_unlock(&frame_cap);
		}
		if(cap_count == 2000)
		{	
			clock_gettime(CLOCK_REALTIME, &stop_time);
			printf("camera stop time is: %d\n", stop_time.tv_sec);
			cap.release();
		}	
		jitter_calculations(thread_id);
		sem_post(&semaphore_arr[0]);
	}
	jitter_final_print(thread_id);
	sem_post(&semaphore_arr[0]);
	pthread_exit(NULL);
}

/* Thread to perform hough eliptical transform*/
void *write_function(void *threadid)
{
	uint8_t thread_id=2;		
 	while(cond)
	{
    		/*Hold semaphore*/
    		sem_wait(&semaphore_arr[thread_id]);
	    	start_arr[thread_id] = calc_ms();
	
		//Do code here
		printf("\n3rd thread");
		

		jitter_calculations(thread_id);
		sem_post(&semaphore_arr[0]);
  	}
	jitter_final_print(thread_id);
	sem_post(&semaphore_arr[0]);
	pthread_exit(NULL);
}

void *thread_4(void *threadid)
{
	uint8_t thread_id=3;		
 	while(cond)
	{
    		/*Hold semaphore*/
    		sem_wait(&semaphore_arr[thread_id]);
	    	start_arr[thread_id] = calc_ms();
	
		//Do code here
		printf("\n4th thread");
	
		jitter_calculations(thread_id);
		sem_post(&semaphore_arr[0]);
  	}
	jitter_final_print(thread_id);
	sem_post(&semaphore_arr[0]);
	pthread_exit(NULL);
}

void *thread_5(void *threadid)
{
	uint8_t thread_id=4;	
 	while(cond)
	{
    		/*Hold semaphore*/
    		sem_wait(&semaphore_arr[thread_id]);
	    	start_arr[thread_id] = calc_ms();
	
		//Do code here
		printf("\n5th thread");
	
		jitter_calculations(thread_id);
		sem_post(&semaphore_arr[0]);
  	}
	jitter_final_print(thread_id);
	sem_post(&semaphore_arr[0]);
	pthread_exit(NULL);
}

void *thread_6(void *threadid)
{
	uint8_t thread_id=5;		
 	while(cond)
	{
    		/*Hold semaphore*/
    		sem_wait(&semaphore_arr[thread_id]);
	    	start_arr[thread_id] = calc_ms();
	
		//Do code here
		printf("\n6th thread");	

		jitter_calculations(thread_id);
		sem_post(&semaphore_arr[0]);
	}
	jitter_final_print(thread_id);
	sem_post(&semaphore_arr[0]);
	pthread_exit(NULL);
}

void *thread_7(void *threadid)
{
	uint8_t thread_id=6;		
 	while(cond)
	{
    		/*Hold semaphore*/
    		sem_wait(&semaphore_arr[thread_id]);
	    	start_arr[thread_id] = calc_ms();
	
		//Do code here
		printf("\n7th thread");	

	
		jitter_calculations(thread_id);
		sem_post(&semaphore_arr[0]);
  	}
	jitter_final_print(thread_id);
	sem_post(&semaphore_arr[0]);
	pthread_exit(NULL);
}

void *thread_8(void *threadid)
{
	uint8_t thread_id=7;	
 	while(cond)
	{
    		/*Hold semaphore*/
    		sem_wait(&semaphore_arr[thread_id]);
	    	start_arr[thread_id] = calc_ms();
	
		//Do code here
		printf("\n8th thread");	

	
		jitter_calculations(thread_id);
		sem_post(&semaphore_arr[0]);
  	}
	jitter_final_print(thread_id);
	sem_post(&semaphore_arr[0]);
	pthread_exit(NULL);
}


/*The main function*/
int main(int argc, char** argv)
{
	clock_gettime(CLOCK_REALTIME, &start_time);
	printf("The start time is %d seconds and %d nanoseconds\n", start_time.tv_sec, start_time.tv_nsec);
	func_arr[0] = sequencer;
  	func_arr[1] = frame_function;
  	func_arr[2] = write_function;
  	func_arr[3] = thread_4;
  	func_arr[4] = thread_5;
	func_arr[5] = thread_6;
	func_arr[6] = thread_7;
	func_arr[7] = thread_8;
	printf("starting threads init\n");
	pthread_mutex_init(&frame_cap, NULL);
 	threads_init();
	if(pthread_mutex_destroy(&frame_cap)!=0)
		perror("mutex A destroy");
	//if(pthread_mutex_destroy(&frame_cap)!=0)
	//	perror("mutex A destroy");
	//if(pthread_mutex_destroy(&frame_cap)!=0)
		//perror("mutex A destroy");
	printf("\nAll done\n");
}

	
