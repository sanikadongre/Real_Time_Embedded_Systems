#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <time.h>
#include <sys/param.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <stdbool.h>
#include <sstream>
#include <fstream>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <opencv/highgui.h>
#include <opencv2/opencv.hpp>
//#include <opencv2/videoio.hpp>
#include <mqueue.h>
#include <iomanip>

using namespace cv;
using namespace std;

#define HRES 640
#define VRES 480
#define MSEC 1000000
#define NSEC_PER_SEC (1000000000)
#define frames_count 10
#define threads_count 7
#define OK (0)

#define mq_ppm "/ppm_writer_mq"
#define mq_jpg "/jpg_writer_mq"
#define ERROR -1

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

double initial_time;
int device =0;
VideoCapture cap(1);
static mqd_t frame_mqueue;
static mqd_t jpg_mqueue;
struct mq_attr attr_frame;
struct mq_attr attr_jpg;

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
	uint8_t i=0,max_priority=20;
	//max_priority = sched_get_priority_max(SCHED_FIFO);
	printf("\nCreating Threads");
	for(i=0;i<threads_count;i++)
	{
		pthread_attr_init(&attr_arr[i]);
		pthread_attr_setinheritsched(&attr_arr[i],PTHREAD_EXPLICIT_SCHED);
		pthread_attr_setschedpolicy(&attr_arr[i],SCHED_FIFO);
		param_arr[i].sched_priority=max_priority-i;	
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
	cout<<"\n\rjoining Threads";
	for(i=0;i<threads_count;i++)
	{
  		pthread_join(thread_arr[i],NULL);
	}
}
 
void generate_mqueue(void)
{
	attr_frame.mq_maxmsg = frames_count;
	attr_frame.mq_msgsize = sizeof(char *);
	attr_frame.mq_flags = 0;
        frame_mqueue= mq_open(mq_ppm, O_CREAT|O_RDWR, 0644, &attr_frame);
        if(frame_mqueue == (mqd_t)ERROR)
        perror("mq_open");
        attr_jpg.mq_maxmsg = frames_count;
	attr_jpg.mq_msgsize = sizeof(char *);
	attr_jpg.mq_flags = 0;
        jpg_mqueue= mq_open(mq_jpg, O_CREAT|O_RDWR, 0644, &attr_jpg);
        if(jpg_mqueue == (mqd_t)ERROR)
        perror("mq_open");
}

void destroy_mqueue(void)
{
	mq_close(frame_mqueue);
	mq_close(jpg_mqueue);
	mq_unlink(mq_ppm);
	mq_unlink(mq_jpg);
}	

void *frame_function(void* ptr)
{
	uint8_t thread_id=0;
        Mat ppm_frame;
        char *ptr_frame;
        char buffer[sizeof(char *)];
        ptr_frame = (char*) malloc(sizeof(ppm_frame.data));
        system("uname -a > output_file.out");	
 	while(counter_arr[thread_id] < frames_count)
  	{
    		/*Hold semaphore*/
    		sem_wait(&semaphore_arr[thread_id]);
	    	start_arr[thread_id] = calc_ms();
	
		cap.open(device);
                cap >> ppm_frame;
                cap.release();
		ptr_frame = (char*) ppm_frame.data;
                memcpy(buffer, &ptr_frame, sizeof(char*));
		if(ptr_frame==NULL)
                {
			printf("Null pointer\n");
			break;
                }
 		//if(mq_send(frame_mqueue, buffer, attr_frame.mq_msgsize, 30) == ERROR)
		//{
			//perror("mq_send error");
		//}
		//if(mq_send(jpg_mqueue, buffer, attr_jpg.mq_msgsize, 30) == ERROR)
		//{
		//	perror("mq_send error");
		//}
		jitter_calculations(thread_id);
		sem_post(&semaphore_arr[thread_id+1]);
  	}
	jitter_final_print(thread_id);
	pthread_exit(NULL);
}

void *write_function(void *ptr)
{
  	uint8_t thread_id=1;	
	Mat ppm_frame;
	unsigned int prio;
	char* ptr_frame;
	char buffer[sizeof(char *)];
	std::ostringstream name;
	std::vector<int> compression_params;
	compression_params.push_back(CV_IMWRITE_PXM_BINARY);
	compression_params.push_back(1);
 	while(counter_arr[thread_id] < frames_count)
  	{
    		/*Hold semaphore*/
    		sem_wait(&semaphore_arr[thread_id]);
	    	start_arr[thread_id] = calc_ms();
	
		/*if(mq_receive(frame_mqueue, buffer, attr_frame.mq_msgsize, &prio) == ERROR)
		{
			perror("mq_receive error");
		}
		else
		{
			memcpy(&ptr_frame, buffer, sizeof(char *));
			ppm_frame = Mat(480,640, CV_8UC3, ptr_frame);
		}*/
		imwrite(name.str(),ppm_frame,compression_params);
		std::fstream outfile;
		std::fstream test;
		std::fstream test1;
		outfile.open(name.str(), ios::in|ios::out);
		outfile.seekp(ios::beg);
		outfile << " ";
		test.open("results_out.txt", ios::in|ios::out|ios::trunc);
		test << outfile.rdbuf();
		outfile.close();
		test.close();
		outfile.open(name.str(), ios::in|ios::out|ios::trunc);
		test1.open("results_out.txt", ios::in|ios::out);
		test.open("output_file.out", ios::in);
		outfile.close();
		test.close();
		jitter_calculations(thread_id);
		sem_post(&semaphore_arr[thread_id+1]);
		name.str(" ");
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
int main(int argc, char* argv[])
{
	if(argc > 1)
	{
		sscanf(argv[1], "%d", &device);
		printf("Using %s\n", argv[1]);
        }
	else if(argc == 1)
	{
		printf("Using default\n");
	}
        else
	{
		printf("Usage: videothread[device]\n");
		exit(-1);
         }
	cap.set(CV_CAP_PROP_FRAME_HEIGHT, 640);
	cap.set(CV_CAP_PROP_FRAME_WIDTH, 480);
	cap.set(CV_CAP_PROP_FPS, 10.0);
        printf("fps %lf\n", cap.get(CV_CAP_PROP_FPS));
  	func_arr[0] = frame_function;
  	func_arr[1] = write_function;
  	func_arr[2] = jpeg_function;
  	func_arr[3] = thread_4;
  	func_arr[4] = thread_5;
	func_arr[5] = thread_6;
	func_arr[6] = thread_7;
	printf("starting threads init\n");
 	threads_init();
	printf("\nAll done\n");
}
