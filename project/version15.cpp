/************************************
*version15.cpp
*Date: 08/12/2018
* Sanika Dongre
* Reference: Sequenecer: Sam Siewert 
*
***********************************/
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
#define frames_count  10
#define threads_count 8
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
static struct timespec cap_start_time = {0,0};
static struct timespec cap_stop_time = {0,0};
static struct mq_attr frame_mq_attr;
double initial_time;
sem_t ppm_sem, jpg_sem, jpg_fin_sem, ppm_fin_sem, camera_sem, ts_sem, ts1_sem;


/*****************************************
*calc_ms: Function to calculate ms value
*It uses clock get real time to calculate 
* time in ms
*
******************************************/
double calc_ms(void)
{
	struct timespec scene = {0,0};
	clock_gettime(CLOCK_REALTIME, &scene);
	return ((scene.tv_sec*1000)+scene.tv_nsec/MSEC);
}

/************************************************
*Jitter calculations: Function for calculating 
*the jitter: the avg difference array, 
*the jitter calculation, the accumulated jitter, 
*the average jitter and the
*accumulated jitter.
*The jitter is calculated for every thread
********************************************/
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
		wcet_arr[thread_id] = run_time[thread_id]; //Worst case jitter
		cout<<"\n\r The worst execution time for thread"<<thread_id+0<<" ="<<wcet_arr[thread_id];
	}
	if(counter_arr[thread_id] == 0)
	{
      		avg_diff_arr[thread_id] = run_time[thread_id]; //To get the average difference array
		printf(" the avg difference array is: %0.8lf ms\n", avg_diff_arr[thread_id]);
    	}
	else if(counter_arr[thread_id] > 0)
	{
		jitter_calc_arr[thread_id] = run_time[thread_id] - avg_diff_arr[thread_id];
		avg_diff_arr[thread_id] = (avg_diff_arr[thread_id] * (counter_arr[thread_id]-1) + run_time[thread_id])/(counter_arr[thread_id]);
  		printf(" The average difference array is: %0.8lf ms\n ", avg_diff_arr[thread_id]);    		
		printf("\rThe calculated jitter is %0.8lf ms\n", jitter_calc_arr[thread_id]); //To get the calculated jitter
		acc_jitter_arr[thread_id] += jitter_calc_arr[thread_id]; //The accumulated jitter is calculated
	}
	counter_arr[thread_id]++;
}
/************************************************
*Function for printing the final jitter values
* ***********************************************/
void jitter_final_print(uint8_t thread_id)
{
	cout<<"\n\rThe accumulated jitter for thread "<<thread_id+0<<" ="<<acc_jitter_arr[thread_id]; 
	cout<<"\n\r The worst execution time for thread"<<thread_id+0<<" ="<<wcet_arr[thread_id];
  	avg_jitter_arr[thread_id]=acc_jitter_arr[thread_id]/frames_count;
	cout<<"\n\rThe average jitter for thread in ms thread"<<thread_id+0<<" ="<<avg_jitter_arr[thread_id]; 

}
/*******************************************************************
*Function: delta_t for calculating the difference between two times
*It has three timespec structures as the parameters to the function
********************************************************************/

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
/******************************************
*Function: For creating the threads, 
*initializing the semaphores and joining the 
*threads
******************************************/
void threads_init(void)
{
	uint8_t i=0; 
	uint8_t max_priority = sched_get_priority_max(SCHED_FIFO); 
	cpu_set_t cpu1,cpu2,cpu3;
	CPU_ZERO(&cpu1);
	CPU_SET(0, &cpu1);
	CPU_ZERO(&cpu2);
	CPU_SET(0, &cpu2);
	CPU_ZERO(&cpu3);
	CPU_SET(0, &cpu3);
	printf("\nCreating Threads");
	for(i=0;i<threads_count;i++)
	{
		pthread_attr_init(&attr_arr[i]);
		pthread_attr_setinheritsched(&attr_arr[i],PTHREAD_EXPLICIT_SCHED);
		pthread_attr_setschedpolicy(&attr_arr[i],SCHED_FIFO);
		param_arr[i].sched_priority=max_priority-i;
		pthread_attr_setaffinity_np(&attr_arr[0], sizeof(cpu1), &cpu1);
		pthread_attr_setaffinity_np(&attr_arr[1], sizeof(cpu2), &cpu2);
		pthread_attr_setaffinity_np(&attr_arr[2], sizeof(cpu3), &cpu3);
		pthread_attr_setaffinity_np(&attr_arr[3], sizeof(cpu3), &cpu3);
		pthread_attr_setaffinity_np(&attr_arr[4], sizeof(cpu3), &cpu3);
		pthread_attr_setaffinity_np(&attr_arr[5], sizeof(cpu3), &cpu3);
				
		if (sem_init (&semaphore_arr[i], 0, 0))
		{
			cout<<"\n\rFailed to initialize semaphore for thread"<<i;
			exit (-1);
		}
		if(sem_init(&ppm_sem, 0,1))
		{
			cout<<"\n\rFailed to initialize semaphore for thread";
			exit(-1);
		}
		if(sem_init(&jpg_sem, 0,1))
		{
			cout<<"\n\rFailed to initialize semaphore for thread";
			exit(-1);
		}
		if(sem_init(&ppm_fin_sem, 0,0))
		{
			cout<<"\n\rFailed to initialize semaphore for thread";
			exit(-1);
		}
		if(sem_init(&jpg_fin_sem, 0,0))
		{
			cout<<"\n\rFailed to initialize semaphore for thread";
			exit(-1);
		}
		if(sem_init(&camera_sem, 0,1))
		{
			cout<<"\n\rFailed to initialize semaphore for thread";
			exit(-1);
		}
		if(sem_init(&ts_sem, 0,1))
		{
			cout<<"\n\rFailed to initialize semaphore for thread";
			exit(-1);
		}
		if(sem_init(&ts1_sem, 0,0))
		{
			cout<<"\n\rFailed to initialize semaphore for thread";
			exit(-1);
		}
		cout<<"\nSemaphore "<<i+0<<" initialized"; 
		cout<<"\nPPM semaphore initialized";
        	pthread_attr_setschedparam(&attr_arr[i], &param_arr[i]);
		if(pthread_create(&thread_arr[i], &attr_arr[i], func_arr[i], (void *)0) !=0)
		{
			perror("ERROR; pthread_create:");
			exit(-1);
		}
		cout<<"\nthread "<<i+0<<" created";
	}
	
	sem_post(&semaphore_arr[1]);
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

/********************************************************
*Sequencer function to send the threads at proper rates
* The semaphores are posted for each thread based on the
*sequencer rates
**********************************************************/

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
			printf("%d seq_loop\n", delay_cnt);
			if((seqCnt % 30) == 0)
			{
				time_check();
				sem_post(&semaphore_arr[1]);
				
			}
			if((seqCnt % 30) == 0)
			{
				
				sem_post(&semaphore_arr[2]);
				
			}
		  	if((seqCnt % 30) == 0)
			{
				
				sem_post(&semaphore_arr[3]);
				
			}
		  	if((seqCnt % 30) == 0)
			{
				sem_post(&semaphore_arr[4]);
				
			}
			if((seqCnt % 30) == 0)
			{
				sem_post(&semaphore_arr[5]);
				
			}
			if((seqCnt % 30) == 0)
			{
				sem_post(&semaphore_arr[6]);
				
			}
			if((seqCnt % 300) == 0)
			{
				sem_post(&semaphore_arr[7]);
				
			}
		}
		sem_post(&semaphore_arr[0]);

	}while(cond);
	for(i=1;i<threads_count;i++)
	{
		sem_post(&semaphore_arr[i]);
		
	}
	pthread_exit(NULL);
}
/****************************************************
*Frame function: The function used to capture frames
*from the camera
****************************************************/
void *frame_function(void *threadid)
{
	
  	uint8_t thread_id=1;	
	
	system("uname -a > system.out");
 	while(cond)
  	{
    		/*Hold semaphore*/
    		sem_wait(&semaphore_arr[thread_id]);
	    	start_arr[thread_id] = calc_ms();
		clock_gettime(CLOCK_REALTIME, &cap_start_time);
		sem_wait(&ppm_sem);
		cap >> ppm_frame; //To get the next frames
		sem_post(&ppm_sem);
		clock_gettime(CLOCK_REALTIME, &cap_stop_time);
		diff= ((cap_stop_time.tv_sec - cap_start_time.tv_sec)*1000000000 + (cap_stop_time.tv_nsec - cap_start_time.tv_nsec)); //To get the difference between capture stop time and capture start time
		printf("\n\r frame capture time is: %0.8lf ns\n", diff);
		frame_ptr = (uint8_t*) ppm_frame.data;
		jitter_calculations(thread_id);
		
	}
	jitter_final_print(thread_id);
	pthread_exit(NULL);
}
/*******************************************************************
*Write Function: is used for writing the ppm images to the disk
*The imwrite OpenCv function is used for writing the ppm images
*******************************************************************/
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
	    	start_arr[thread_id] = calc_ms();
		printf("\n3rd thread\n");
		name.str("frame_");
		name<<"frame_"<<counter_arr[thread_id]<<".ppm";
		time (&rawtime);
 		timecur = localtime (&rawtime);
		putText(ppm_frame,asctime(timecur),Point(465,470),FONT_HERSHEY_COMPLEX_SMALL,0.7,Scalar(255,255,0),2);
		imwrite(name.str(), ppm_frame, compression_params);
		name.str(" ");
		jitter_calculations(thread_id);
		sem_post(&ppm_fin_sem); //semaphore to indicate ppm write is done
		sem_post(&ts1_sem);
		sem_post(&ppm_sem);
		
  	}
	jitter_final_print(thread_id);
	pthread_exit(NULL);
}
/********************************************
*Jpg function: It is a function for converting 
*the ppm images into jpg(compressed images)
*It reads the ppm images and converts them 
*into jpg images
********************************************/
void *jpg_function(void *threadid)
{
	
	uint8_t thread_id=3;	
	ostringstream name;

	vector<int> compression_params;
	compression_params.push_back(CV_IMWRITE_JPEG_QUALITY);
	compression_params.push_back(95);	
 	while(cond)
	{
    		/*Hold semaphore*/
		
    		sem_wait(&semaphore_arr[thread_id]);
		sem_wait(&ppm_fin_sem);
		start_arr[thread_id] = calc_ms();
		printf("\n4th thread\n");
		name.str("frame_");
		name<<"frame_"<<counter_arr[thread_id]<<".ppm";
		frame_jpg = imread(name.str(),CV_LOAD_IMAGE_COLOR); //To read the ppm image and store it as Mat
		name.str("");
		name<<"frame_"<<counter_arr[thread_id]<<".jpg";
		imwrite(name.str(), frame_jpg, compression_params); //To write the jpg images to the disk
		name.str(" ");
		jitter_calculations(thread_id);
		sem_post(&jpg_sem);
  	}
	jitter_final_print(thread_id);
	pthread_exit(NULL);
}
/***************************************************
*Timestamp function to add timestamps embedded in the
*PPM header for each frame and also the timestamps 
*for the system version of the raspberry pi 
*embedded in the ppm header
****************************************************/

void *timestamp_function(void *threadid)
{
	uint8_t thread_id=4;	
	fstream input_file, output_file, ts;
	ostringstream name, name_1;
 	while(cond)
	{
    		/*Hold semaphore*/
    		sem_wait(&semaphore_arr[thread_id]);
		sem_wait(&ts_sem);
		sem_wait(&ts1_sem);
	    	start_arr[thread_id] = calc_ms();
		name.str(" ");
		name_1.str(" ");
		name<<"frame_"<<counter_arr[thread_id]<<".ppm";
		name_1<<"output_"<<counter_arr[thread_id]<<".ppm";
		input_file.open(name.str(), ios::in);
		output_file.open(name_1.str(), ios::out);
		ts.open("system.out", ios::in);
		output_file << "P6" << endl << "#Timestamp:" << asctime(timecur) << "#System:" << ts.rdbuf() << "#" << input_file.rdbuf(); //To add timestamp in the output file
		output_file.close();
		input_file.close();
		ts.close();
		printf("\n5th thread");
		jitter_calculations(thread_id);
		
		sem_post(&ts_sem);
  	}
	jitter_final_print(thread_id);
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
		printf("\n6th thread");	
		jitter_calculations(thread_id);
		
	}
	jitter_final_print(thread_id);
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
		printf("\n7th thread");	
		jitter_calculations(thread_id);
		
  	}
	jitter_final_print(thread_id);
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
		printf("\n8th thread");	
		jitter_calculations(thread_id);
		
  	}
	jitter_final_print(thread_id);
	pthread_exit(NULL);
}


/*The main function*/
int main(int argc, char *argv[])
{
	clock_gettime(CLOCK_REALTIME, &start_time); /*To find the start time for the code*/
	printf("\nThe start time is %d seconds and %d nanoseconds\n", start_time.tv_sec, start_time.tv_nsec);

	if(argc > 1)
	{
		sscanf(argv[1], "%d", &device);
	}
	
	cap.set(CV_CAP_PROP_FRAME_WIDTH, HRES); //It sets the width of the image viewer resolution
	cap.set(CV_CAP_PROP_FRAME_HEIGHT, VRES); // it sets the
	cap.set(CV_CAP_PROP_FPS,10.0); //For setting the camera frame rate
	cap.open(device); //Camera start
	printf("fps %lf\n", cap.get(CV_CAP_PROP_FPS));
	func_arr[0] = sequencer;
  	func_arr[1] = frame_function;
  	func_arr[2] = write_function;
  	func_arr[3] = jpg_function;
  	func_arr[4] = timestamp_function;
	func_arr[5] = thread_6;
	func_arr[6] = thread_7;
	func_arr[7] = thread_8;
	printf("starting threads init\n");
 	threads_init(); //For creating and joining threads
	cap.release(); //To release the camera
	clock_gettime(CLOCK_REALTIME, &stop_time);
	printf("\nThe code stop time is %d seconds and %d nanoseconds\n",stop_time.tv_sec, stop_time.tv_nsec); //To find the stop time for the code*/
	delta_t(&stop_time, &start_time, &exe_time); //To calculate the execution time of the code for specified number of frames*/
	cout<<"\n\r The execution time for the code is: "<<exe_time.tv_sec<<" seconds "<<exe_time.tv_nsec<<" nano seconds.\n"; /*To print out the execution time of the code*/
	printf("\nAll done\n");
}

	
