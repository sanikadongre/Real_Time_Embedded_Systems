#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <time.h>
#include <sys/param.h>
#include <pthread.h>
#include <semaphore.h>
#include <syslog.h>
#include <sys/time.h>
#include <stdbool.h>
#include <errno.h>

using namespace std;

#define HRES 640
#define VRES 480
#define MSEC 1000000
#define NSEC_PER_SEC (1000000000)
#define frames_count 10
#define threads_count 7
#define OK (0)

double framerate;
double value;


sem_t semaphore_arr[threads_count];
pthread_t thread_arr[threads_count];
pthread_attr_t attr_arr[threads_count];
struct sched_param param_arr[threads_count];
void* (*func_arr[threads_count]) (void*) ;
double start_arr[threads_count] = {0,0,0,0,0,0,0};
double stop_arr[threads_count] = {0,0,0,0,0,0,0};
double difference_arr[threads_count] = {0,0,0,0,0,0,0};
double acc_jitter_arr[threads_count] = {0,0,0,0,0,0,0};
double avg_jitter_arr[threads_count] = {0,0,0,0,0,0,0};
uint32_t counter_arr[threads_count] = {0,0,0,0,0,0,0};

typedef struct
{
	int threadIdx;
	unsigned long long sequencePeriods;
}threadParams_t;

double initial_time;
struct timeval start_time_val;

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
	uint8_t i=0,j=0;
	uint8_t max_priority = sched_get_priority_max(SCHED_FIFO);
	printf("\nCreating Threads");
	for(i=0;i<threads_count;i++)
	{
		pthread_attr_init(&attr_arr[i]);
		pthread_attr_setinheritsched(&attr_arr[i],PTHREAD_EXPLICIT_SCHED);
		pthread_attr_setschedpolicy(&attr_arr[i],SCHED_FIFO);
                for(j=0; j< ((threads_count-1)/2); j++)
		{
			param_arr[j].sched_priority=max_priority-i;
                }
                      param_arr[4].sched_priority = max_priority-3;
                      param_arr[5].sched_priority = max_priority -2;
                      param_arr[6].sched_priority = max_priority-3;
		      param_arr[7].sched_priority = max_priority-2;	
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
	sem_post(&semaphore_arr[0]);
        //threadParams[0].sequencePeriods = 900;
	cout<<"\n\rjoining Threads";
	for(i=0;i<threads_count;i++)
	{
  		pthread_join(thread_arr[i],NULL);
	}
}

void *frame_function(void* ptr)
{
	uint8_t thread_id=0;	
 	while(counter_arr[thread_id] < frames_count)
  	{
    		/*Hold semaphore*/
    		sem_wait(&semaphore_arr[thread_id]);
	    	start_arr[thread_id] = calc_ms();
	
		//Do code here
	
		jitter_calculations(thread_id);
		sem_post(&semaphore_arr[thread_id+1]);
  	}
	jitter_final_print(thread_id);
	pthread_exit(NULL);
}

void *write_function(void *ptr)
{
  	uint8_t thread_id=1;	
 	while(counter_arr[thread_id] < frames_count)
  	{
    		/*Hold semaphore*/
    		sem_wait(&semaphore_arr[thread_id]);
	    	start_arr[thread_id] = calc_ms();
	
		//Do code here
	
		jitter_calculations(thread_id);
		sem_post(&semaphore_arr[thread_id+1]);
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
		sem_post(&semaphore_arr[thread_id+1]);
  	}
	jitter_final_print(thread_id);
	pthread_exit(NULL);
}

void *sequencer_function(void *threadp)
{
  uint8_t thread_id=3;	
  int abortTest = FALSE;
  int abortS0 = FALSE, abortS1 = FALSE, abortS2 = FALSE,
  abortS3 = FALSE, abortS4 = FALSE, abortS5 = FALSE, abortS6 = FALSE;	
  struct timeval current_time_val;
  struct timespec delay_time = {0,33333333};
  struct timespec remaining_time;
  double current_time;
  double residual;
  int rc, delay_cnt=0;
  unsigned long long seqCnt=0;
  threadParams_t* threadParams = (threadParams_t *)threadp;
  gettimeofday(&current_time_val, (struct timezone *)0));
  printf("Sequencer thread @ sec=%d, msec=%d\n", (int)(current_time_val.tv_sec-start_time_val.tv_sec), (int)current_time_val.tv_usec/1000000);
 	while(counter_arr[thread_id] < frames_count)
  	{
    		/*Hold semaphore*/
    		sem_wait(&semaphore_arr[thread_id]);
	    	start_arr[thread_id] = calc_ms();
	
		do
		{
			delay_cnt =0;
			residual=0.0;
			//gettimeofday(&current_time_val, (struct timezone *)0);
			do
 			{
			  rc= nanosleep(&delay_time, &remaining_time);
			  if(rc== EINTR)
				{
				   residual = remaining_time.tv_sec + ((double) remaining_time.tv_nsec / 1000000000);
				   delay_cnt++;
                                 }
                          else if(rc < 0)
				{
					perror("Sequencer nanosleep");
					exit(-1);
                                 }
			} while((residual > 0.0) && (delay_cnt <100));
                        seqCnt++;
			gettimeofday(&current_time_val, ((struct timezone *)0);
			if(delay_cnt >1) printf("Sequencer looping delay %d\n", delay_cnt);
			if((seqCnt % 10) == 0)
			sem_post(&semaphore_arr[0]);
			if((seqCnt % 30) == 0)
			sem_post(&semaphore_arr[1]);
			if((seqCnt % 60) == 0)
			sem_post(&semaphore_arr[2]);
			if((seqCnt % 30)== 0)
			sem_post(&semaphore_arr[3]);
			if((seqCnt % 60) ==0)
			sem_post(&semaphore_arr[4]);
			if((seqCnt % 30) == 0)
			sem_post(&semaphore_arr[5]);
			if((seqCnt % 300) == 0)
			sem_post(&semaphore_arr[6]);
			//gettimeofday(&current_time_val, (struct timezone *)0);
			} while(!abortTest && (seqCnt < threadParams -> sequencePeriods));
			sem_post(&semaphore_arr[0]);
			sem_post(&semaphore_arr[1]);
			sem_post(&semaphore_arr[2]);
			sem_post(&semaphore_arr[3]);
			sem_post(&semaphore_arr[4]);
			sem_post(&semaphore_arr[5]);
			sem_post(&semaphore_arr[6]);

		        abortS0 = TRUE;
                        abortS1 = TRUE;
			abortS2 = TRUE;
                        abortS3 = TRUE;
			abortS4 = TRUE;
                        abortS5 = TRUE;
			abortS6 = TRUE;
	
	
		jitter_calculations(thread_id);
		sem_post(&semaphore_arr[thread_id+1]);
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
		sem_post(&semaphore_arr[thread_id+1]);
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
		sem_post(&semaphore_arr[thread_id+1]);
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
		sem_post(&semaphore_arr[0]);
  	}
	jitter_final_print(thread_id);
	pthread_exit(NULL);
}
/*The main function*/
int main(int argc, char** argv)
{
	struct timeval current_time_val;
  	func_arr[0] = frame_function;
  	func_arr[1] = write_function;
  	func_arr[2] = jpeg_function;
  	func_arr[3] = sequencer_function;
  	func_arr[4] = thread_5;
	func_arr[5] = thread_6;
	func_arr[6] = thread_7;
	printf("starting threads init\n");
 	threads_init();
	printf("\nAll done\n");
}
