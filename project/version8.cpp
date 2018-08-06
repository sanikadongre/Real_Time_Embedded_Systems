#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <time.h>
#include <sys/param.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>

using namespace std;

#define HRES 640
#define VRES 480
#define MSEC 1000000
#define NSEC_PER_SEC (1000000000)
#define frames_count 10
#define threads_count 8
#define OK (0)

double framerate;
double value;
typedef struct
{
	unsigned long long sequencePeriods;
}threadParams_t;


sem_t semaphore_arr[threads_count];
pthread_t thread_arr[threads_count];
pthread_attr_t attr_arr[threads_count];
struct sched_param param_arr[threads_count];
void* (*func_arr[threads_count]) (void*) ;
double start_arr[threads_count] = {0,0,0,0,0,0,0,0};
double stop_arr[threads_count] = {0,0,0,0,0,0,0,0};
double difference_arr[threads_count] = {0,0,0,0,0,0,0,0};
double acc_jitter_arr[threads_count] = {0,0,0,0,0,0,0,0};
double avg_jitter_arr[threads_count] = {0,0,0,0,0,0,0,0};
uint32_t counter_arr[threads_count] = {0,0,0,0,0,0,0,0};
struct timespec start_time, stop_time, exe_time;

double initial_time;

double calc_ms(void)
{
	struct timespec scene = {0,0};
	clock_gettime(CLOCK_REALTIME, &scene);
	return (((scene.tv_sec)*1000.0)+((scene.tv_nsec)/MSEC));
}

void jitter_calculations(uint8_t thread_id)
{
	double run_time=0.0, jitter_calc=0.0;
	cout<<"\n\rThread"<<thread_id+0;
 	printf("\n\rframe: %d\n",counter_arr[thread_id]);
    	printf("\n\rstart time in ms is: %0.8lf ms \n", start_arr[thread_id]); 
    	stop_arr[thread_id] = calc_ms();
    	printf("\n\r stop time in ms is: %0.8lf ms\n", stop_arr[thread_id]);
    	run_time = stop_arr[thread_id] - start_arr[thread_id];
    	if(counter_arr[thread_id]>0)
    	{
      		jitter_calc = run_time - difference_arr[thread_id];
      		printf("\rThe calculated jitter is %0.8lf ms\n", jitter_calc);
		difference_arr[thread_id] = run_time;
    	}
    	acc_jitter_arr[thread_id] += jitter_calc;
	counter_arr[thread_id]++;
}

void jitter_final_print(uint8_t thread_id)
{
	//cout<<"\n\rThe accumulated jitter for thread "<<thread_id+0<<" ="<<acc_jitter_arr[thread_id];
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
	//sem_post(&semaphore_arr[1]);
	printf("SANU\n");
	cout<<"\n\rjoining Threads";
	for(i=0;i<threads_count;i++)
	{
  		pthread_join(thread_arr[i],NULL);
	}
}
void *sequencer(void *threadid)
{
	uint8_t thread_id = 0;	
	struct timespec delay_time = {0, 33333333};
	struct timespec remaining_time;
	struct timespec current_time_val;
	unsigned long long seqCnt =0; double current_time;
	double residual; int rc; int delay_cnt =0;
	clock_gettime(CLOCK_REALTIME, &stop_time);
	printf("The stop time is %d seconds and %d nanoseconds\n", stop_time.tv_sec, stop_time.tv_nsec);
	
	do 
	{       sem_post(&semaphore_arr[0]);
		sem_wait(&semaphore_arr[0]);

		delay_cnt=0; residual=0.0;
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
		if(delay_cnt > 1)
		printf("%d seq_loop\n", delay_cnt);
		 if((seqCnt % 10) == 0)
			sem_post(&semaphore_arr[1]);
		  if((seqCnt % 30) == 0)
			sem_post(&semaphore_arr[2]);
		  if((seqCnt % 60) == 0)
			sem_post(&semaphore_arr[3]);
		  if((seqCnt % 30) == 0)
			sem_post(&semaphore_arr[4]);
		  if((seqCnt % 60) == 0)
			sem_post(&semaphore_arr[5]);
		  if((seqCnt % 30) == 0)
			sem_post(&semaphore_arr[6]);
		  if((seqCnt % 300) == 0)
			sem_post(&semaphore_arr[7]);

		  }while((1));
			sem_post(&semaphore_arr[1]);
			sem_post(&semaphore_arr[2]);
			sem_post(&semaphore_arr[3]);
			sem_post(&semaphore_arr[4]);
			sem_post(&semaphore_arr[5]);
			sem_post(&semaphore_arr[6]);
			sem_post(&semaphore_arr[7]);
	pthread_exit(NULL);
}

void *write_function(void *threadid)
{
  	uint8_t thread_id=1;	
 	while(counter_arr[thread_id] < frames_count)
  	{
    		/*Hold semaphore*/
    		sem_wait(&semaphore_arr[thread_id]);
	    	start_arr[thread_id] = calc_ms();
	
		//Do code here
	
		jitter_calculations(thread_id);
		//sem_post(&semaphore_arr[thread_id+1]);
  	}
	jitter_final_print(thread_id);
	pthread_exit(NULL);
}

/* Thread to perform hough eliptical transform*/
void *jpeg_function(void *threadid)
{
  uint8_t thread_id=2;	
 	while(counter_arr[thread_id] < frames_count)
  	{
    		/*Hold semaphore*/
    		sem_wait(&semaphore_arr[thread_id]);
	    	start_arr[thread_id] = calc_ms();
	
		//Do code here
	
		jitter_calculations(thread_id);
		//sem_post(&semaphore_arr[thread_id+1]);
  	}
	jitter_final_print(thread_id);
	pthread_exit(NULL);
}

void *thread_4(void *threadid)
{
  uint8_t thread_id=3;	
 	while(counter_arr[thread_id] < frames_count)
  	{
    		/*Hold semaphore*/
    		sem_wait(&semaphore_arr[thread_id]);
	    	start_arr[thread_id] = calc_ms();
	
		//Do code here
	
		jitter_calculations(thread_id);
		//sem_post(&semaphore_arr[thread_id+1]);
  	}
	jitter_final_print(thread_id);
	pthread_exit(NULL);
}

void *thread_5(void *threadid)
{
uint8_t thread_id=4;	
 	while(counter_arr[thread_id] < frames_count)
  	{
    		/*Hold semaphore*/
    		sem_wait(&semaphore_arr[thread_id]);
	    	start_arr[thread_id] = calc_ms();
	
		//Do code here
	
		jitter_calculations(thread_id);
		//sem_post(&semaphore_arr[thread_id+1]);
  	}
	jitter_final_print(thread_id);
	pthread_exit(NULL);
}

void *thread_6(void *threadid)
{
  uint8_t thread_id=5;	
 	while(counter_arr[thread_id] < frames_count)
  	{
    		/*Hold semaphore*/
    		sem_wait(&semaphore_arr[thread_id]);
	    	start_arr[thread_id] = calc_ms();
	
		//Do code here
	
		jitter_calculations(thread_id);
		//sem_post(&semaphore_arr[thread_id+1]);
  	}
	jitter_final_print(thread_id);
	pthread_exit(NULL);
}

void *thread_7(void *threadid)
{
 uint8_t thread_id=6;	
 	while(counter_arr[thread_id] < frames_count)
  	{
    		/*Hold semaphore*/
    		sem_wait(&semaphore_arr[thread_id]);
	    	start_arr[thread_id] = calc_ms();
	
		//Do code here
	
		jitter_calculations(thread_id);
		//sem_post(&semaphore_arr[thread_id+1]);
  	}
	jitter_final_print(thread_id);
	pthread_exit(NULL);
}

void *thread_8(void *threadid)
{
 uint8_t thread_id=7;	
 	while(counter_arr[thread_id] < frames_count)
  	{
    		/*Hold semaphore*/
    		sem_wait(&semaphore_arr[thread_id]);
	    	start_arr[thread_id] = calc_ms();
	
		//Do code here
	
		jitter_calculations(thread_id);
		//sem_post(&semaphore_arr[0]);
  	}
	jitter_final_print(thread_id);
	pthread_exit(NULL);
}


/*The main function*/
int main(int argc, char** argv)
{
	clock_gettime(CLOCK_REALTIME, &start_time);
	printf("The start time is %d seconds and %d nanoseconds\n", start_time.tv_sec, start_time.tv_nsec);
  	threadParams_t threadParams[threads_count];
	threadParams[0].sequencePeriods=900;
	func_arr[0] = sequencer;
  	func_arr[1] = write_function;
  	func_arr[2] = jpeg_function;
  	func_arr[3] = thread_4;
  	func_arr[4] = thread_5;
	func_arr[5] = thread_6;
	func_arr[6] = thread_7;
	func_arr[7] = thread_8;
	printf("starting threads init\n");
 	threads_init();
	printf("\nAll done\n");
}
