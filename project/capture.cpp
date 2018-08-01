#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdbool.h>         
#include <fstream> 
#include <sstream> 
#include <sched.h>          
#include <time.h>
#include <fcntl.h>             
#include <unistd.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <mqueue.h>
#include <iostream>
#include "opencv/core/core.hpp"
#include "opencv/highgui/highgui.hpp"
#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>
#define mq_ppm "/ppm_writer_mq"
#define mq_jpeg "/jpg_writer_mq"
#define ERROR -1
#define threads_count 3
#define frames_count 10
#define mega 1000000
#define thousand 1000
using namespace cv;
using namespace std;
int device=0;
VideoCapture cap(1);

static struct timespec time_end = {0, 0};
static struct timespec time_left = {0, 0};
double start_capture = 0;

static mqd_t mqueue_frame;
static mqd_t mqueue_jpeg;
struct mq_attr mqueue_attr;
struct mq_attr mqueue_jpeg_attr;

typedef struct
{
    int threadid;
} threadParams_t;

pthread_t threads[threads_count];
pthread_attr_t sched_attr[threads_count];
int max_priority, min_priority;
struct sched_param param[threads_count];
struct sched_param main_param;
pthread_attr_t attr_main;
pid_t priority_main;
sem_t semaphore, jpeg_semaphore, semaphore_storing;

double time_func_msec();
void time_func_delay(long int time_sec, long int time_nsec);
void generate_mqueue(void);
void destroy_mqueue(void);

void time_func_delay(long int time_sec, long int time_nsec)
{
   time_end.tv_sec = time_sec;
   time_end.tv_nsec = time_nsec;

  do
  {
    time_nsec(&time_end, &time_left);

    time_end.tv_sec = time_left.tv_sec;
    time_end.tv_nsec = time_left.tv_nsec;
  }
  while ((time_left.tv_sec > 0) || (time_left.tv_nsec > 0));

}

double time_func_msec(void)
{
  struct timespec time_sample = {0, 0};

  clock_gettime(CLOCK_REALTIME, &time_sample);
  double time_val = ((time_sample.tv_sec)*thousand) + ((time_sample.tv_nsec)/mega);
  return time_val;
}

void generate_mqueue(void)
{
  mqueue_attr.mq_maxmsg = frames_count;
  mqueue_attr.mq_msgsize = sizeof(char *);
  mqueue_attr.mq_flags = 0;

  mqueue_frame = mq_open(mq_ppm, O_CREAT|O_RDWR, 0644, &mqueue_attr);
  if(mqueue_frame == (mqd_t)ERROR)
    perror("mq_open");

  mqueue_jpeg_attr.mq_maxmsg = frames_count;
  mqueue_jpeg_attr.mq_msgsize = sizeof(char *);
  mqueue_jpeg_attr.mq_flags = 0;

  mqueue_jpeg = mq_open(mq_jpeg, O_CREAT|O_RDWR, 0644, &mqueue_jpeg_attr);
  if(mqueue_jpeg == (mqd_t)ERROR)
    perror("mq_open");
}


void destroy_mqueue(void)
{
  mq_close(mqueue_frame);
  mq_close(mqueue_jpeg);
  mq_unlink(mq_ppm);
  mq_unlink(mq_jpeg);
}


// THREAD 1: main thread for capturing frames from camera
void *frame_thread(void *threadd)
{
    int f = 0;

    double end_time = 0, run_time = 0, old_time = 0, jitter=0, jitter_acc=0, jitter_avg = 0;
    Mat frame;
    char *frame_pointer;
    char buffer_arr[sizeof(char *)];
    frame_pointer =  (char *) malloc(sizeof(frame.data));

    system("uname -a > output.out");

    while(i < frames_count)
    {
     
        sem_wait(&semaphore);
        start_capture= time_func_msec();
        printf("Frame: %d\n", f);
        printf("Timestamp is: %0.8lf seconds\n", start_capture/thousand);
        cap.open(dev);
        cap >> frame; 
        cap.release();
        frame_pointer = (char *) frame.data;
        memcpy(buffer_arr, &frame_pointer, sizeof(char *));
        if(frame_pointer==NULL) 
       {
          printf("Null pointer\n");
          break;
        }

        if(mq_send(mqueue_frame, buffer_arr, mqueue_attr.mq_msgsize, 30) == ERROR)
        {
          perror("Error in sending message queue for a ppm image\n");
        }

        if(mq_send(mqueue_jpeg, buffer_arr, mqueue_jpeg_attr.mq_msgsize, 30) == ERROR)
        {
          perror("Error in sending message queue for a jpeg image");
        }
        f++;
        end_time = time_func_msec();
        run_time = (end_time - start_capture);
        printf("The run time for capture is: %0.8lf\n", exec_time/thousand);
        if(f>0){
        jitter = run_time - old_time;
        }
        old_time = run_time;
        jitter_acc += jitter;
        sem_post(&semaphore_storing);
        sem_post(&jpeg_semaphore);
    }
    jitter_avg = jitter_acc/frames_count;
    printf("\nAverage jitter for captured frames is: %0.8lf miliseconds\n\n", jitter_avg);
    pthread_exit(NULL);
}
void *thread_write(void *threadd)
{
    int f = 0;
    unsigned int thread_priorities;
    Mat frame;
    char *frame_pointer;
    char buffer_arr[sizeof(char *)];
    std::ostringstream name;
    std::vector<int> compression_params;
    compression_params.push_back(CV_IMWRITE_PXM_BINARY);
    compression_params.push_back(1); 
    while(f < frames_count)
    {
      sem_wait(&semaphore_storing);
      name << "frame_" << f << ".ppm";
      if(mq_receive(mqueue_frame, buffer_arr, mqueue_attr.mq_msgsize, &thread_priorities) == ERROR)
      {
        perror("mq_receive for receiving error\n");
      }
      else
      {
        memcpy(&frame_pointer, buffer_arr, sizeof(char *));
        frame = Mat(480, 640, CV_8UC3, frame_pointer);
      }
      imwrite(name.str(), frame_ppm, compression_params);
      std::fstream outfile;
      std::fstream temp;
      std::fstream temp1;
      outfile.open (name.str(), ios::in|ios::out);
      outfile.seekp (ios::beg);
      outfile << "  ";
      temp.open("results.txt", ios::in|ios::out|ios::trunc);
      temp << outfile.rdbuf();
      outfile.close();
      temp.close();
      outfile.open (name.str(), ios::in|ios::out|ios::trunc);
      temp1.open("results.txt", ios::in|ios::out);
      temp.open("output.out", ios::in);
      outfile << "P6" << endl << "#Prescise Time Stamp is: " << setprecision(10) << fixed << start_capture/thousand << " seconds" << endl << "#System Specs are: " << temp.rdbuf() << temp1.rdbuf();
      outfile.close();
      temp.close();
      f++;
      name.str("");
    }
    pthread_exit(NULL);
}

void *thread_jpeg(void *threadd)
{
    Mat frame_jpeg;
    int f = 0;
    double start_frame = 0, end_frame= 0, run_time=0, old_time=0, jitter=0, jitter_acc=0, jitter_avg=0;
    unsigned int thread_priorities;
    char *frame_pointer;
    char buffer_arr[sizeof(char *)];
    std::ostringstream name, name1;
    std::vector<int> compression_params;
    compression_params.push_back(CV_IMWRITE_JPEG_QUALITY);
    compression_params.push_back(95); // #0 for P3 and #1 for P6

    while(f < frames_count)
    {
        sem_wait(&jpeg_semaphore);
        start_frame = time_func_msec();
        name << "frame_comp" << i << ".jpg";
        name1 << "frame_" << i << ".ppm";
        if(mq_receive(mqueue_jpeg, buffer_arr, mqueue_jpeg_attr.mq_msgsize, &thread_priorities) == ERROR)
        {
          perror("mq_receive error for the jpeg image\n");
        }
        else
        {
          memcpy(&frame_pointer, buffer_arr, sizeof(char *));
          frame_jpeg = Mat(480, 640, CV_8UC3, frame_pointer);
        }
        imwrite(name.str(), frame_jpg, compression_params);
        name.str("");
        name1.str("");
        f++;
        end_frame = time_func_msec();
        run_time = end_frame - start_frame;
        printf("Jpeg run Time is: %0.8lf\n", run_time/thousand);
        if(f>0){
        jitter = run_time - old_time;
        }
        old_time = run_time;
        jitter_acc += jitter;
    }
    jitter_avg = jitter_acc/frames_count;
    printf("\nAverage compress jitter for images is: %0.8lf miliseconds\n\n", jitter_avg);
    pthread_exit(NULL);
}

void *adder(void *threadd)
{
  int f = 0;
  do
  {
      sem_post(&semaphore);
      time_func_delay(1, 0);
      f++;
   }
   while (f<frames_count);
   pthread_exit(NULL);
}


int main(int argc, char *argv[])
{
    if(argc > 1)
    {
        sscanf(argv[1], "%d", &dev);
        printf("Using %s\n", argv[1]);
    }
    else if(argc == 1){
        printf("The original one\n");
    }
    else
    {
        printf(" capture[dev] is displayed\n");
        exit(-1);
    }
    int f = 0, prio;
    cap.set(CV_CAP_PROP_FRAME_HEIGHT, 640);
    cap.set(CV_CAP_PROP_FRAME_WIDTH, 480);
    cap.set(CV_CAP_PROP_FPS, 10.0);
    printf("fps %lf\n", cap.get(CV_CAP_PROP_FPS));
    cpu_set_t cpuset;
    cpu_set_t cpuset1;
    generate_mqueue();
    sem_init(&semaphore, 0, 0);
    sem_init(&semaphore_storing, 0, 0);
    sem_init(&jpeg_semaphore, 0, 0);
    priority_main =getpid();
    max_priority = sched_get_priority_max(SCHED_FIFO);
    min_priority = sched_get_priority_min(SCHED_FIFO);
    prio=sched_getparam(priority_main, &main_param);
    main_param.sched_priority= max_priority;
    prio=sched_setscheduler(getpid(), SCHED_FIFO, &main_param);
    if(prio < 0) 
    {
     perror("main_param error\n");
    }
    CPU_ZERO(&cpuset);
    CPU_SET(1, &cpuset);
    for(f=0;f<threads_count;f++)
    {
        pthread_attr_init(&sched_attr[i]);
    }
    pthread_attr_setaffinity_np(&sched_attr[0], sizeof(cpu_set_t), &cpuset);
    param[0].sched_priority=max_priority-1;
    pthread_attr_setschedparam(&sched_attr[0], &param[0]);
    pthread_create(&threads[0], (&sched_attr[0], adder, (void*) (NULL));
    CPU_ZERO(&cpuset1);
    CPU_SET(2, &cpuset1);
    for(f=1;f<threads_count-1;f++){
      prio=pthread_attr_setinheritsched(&sched_attr[i], PTHREAD_EXPLICIT_SCHED);
      prio=pthread_attr_setschedpolicy(&sched_attr[i], SCHED_FIFO);
      pthread_attr_setaffinity_np(&sched_attr[i], sizeof(cpu_set_t), &cpuset1);
      param[i].sched_priority= max_priority-i-1;
      pthread_attr_setschedparam(&sched_attr[i], &param[i]);
    }
    pthread_create(&threads[1], &sched_attr[1], thread_frame, (void*)(NULL));
    pthread_create(&threads[2], &sched_attr[2], thread_write, (void*)(NULL));
    pthread_create(&threads[3], &sched_attr[3], thread_jpeg, (void*)(NULL));
    for(f=0;f<threads_count;f++)
    {
       pthread_join(threads[f], NULL);
    }
    destroy_mqueue();
    return 0;
}



