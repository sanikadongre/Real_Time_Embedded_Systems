
                                                                   
#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<pthread.h>
#include<mqueue.h>
#include<sys/stat.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<mqueue.h>

#define SNDRCV_MQ "/send_receive_mq_heap"

struct mq_attr mq_attr;				//message queue attributes

static mqd_t mymq;

pthread_t receive_thread, send_thread;	//thread declaration

pthread_attr_t att[2];

struct sched_param parameter[2];		//thread parameters

static char imagebuff[4096];

void *sender_function(void *threadp) //Sender 
{
  char buffer[sizeof(void *)+sizeof(int)];
  void *buffptr;
  int prio;
  int nbytes;
  int id = 999;


  while(1) {

    /* memory is allocated using malloc function and its priority is set as 30 */

    buffptr = (void *)malloc(sizeof(imagebuff));
    strcpy(buffptr, imagebuff);
    printf("\nMessage to send = %s\n", (char *)buffptr);

    printf("\nSending %ld bytes\n", sizeof(buffptr));

    memcpy(buffer, &buffptr, sizeof(void *));
    memcpy(&(buffer[sizeof(void *)]), (void *)&id, sizeof(int));

    if((nbytes = mq_send(mymq, buffer, (size_t)(sizeof(void *)+sizeof(int)), 30)) == -1)
    {
      perror("mq_send");
    }
    else
    {
      printf("\nSend: message ptr 0x%p successfully sent\n", buffptr);
    }

    usleep(3000000);

  }
  
}

void *receiver_function(void *threadp) //Receiver
{
  char buffer[sizeof(void *)+sizeof(int)];
  void *buffptr; 
  int prio;
  int nbytes;
  int count = 0;
  int id;
 
  while(1) {

    /* This is the Highest priority msg from the message queue */

    printf("\nReading %ld bytes\n", sizeof(void *));
  
    if((nbytes = mq_receive(mymq, buffer, (size_t)(sizeof(void *)+sizeof(int)), &prio)) == -1)

    {
      perror("mq_receive");
    }
    else
    {
      memcpy(&buffptr, buffer, sizeof(void *));
      memcpy((void *)&id, &(buffer[sizeof(void *)]), sizeof(int));
      printf("\nReceive: ptr msg 0x%p received with priority = %d, length = %d, id = %d\n", buffptr, prio, nbytes, id);

      printf("\nContents of ptr = \n%s\n", (char *)buffptr);

      free(buffptr);

      printf("\nHeap space memory freed\n");

    }
    
  }

}
static int sid, rid;

void main()
{
  int i, j;
  char pixel = 'A';
  int rc;

  for(i=0;i<4096;i+=64) {
    pixel = 'A';
    for(j=i;j<i+64;j++) {
      imagebuff[j] = (char)pixel++;
    }
    imagebuff[j-1] = '\n';
  }
  imagebuff[4095] = '\0';
  imagebuff[63] = '\0';

  printf("buffer =\n%s", imagebuff);

  mq_attr.mq_maxmsg = 100;
  mq_attr.mq_msgsize = sizeof(void *)+sizeof(int);

  mq_attr.mq_flags = 0;

  /* O_RWDR is used as the second parameter for message queue open and gives read write permission*/
  mymq = mq_open(SNDRCV_MQ, O_CREAT|O_RDWR, 0664, &mq_attr);

for (int i=0; i<2; i++)
{
  rc=pthread_attr_init(&att[i]);
  rc=pthread_attr_setinheritsched(&att[i], PTHREAD_EXPLICIT_SCHED);
  rc=pthread_attr_setschedpolicy(&att[i], SCHED_FIFO);
  parameter[i].sched_priority=99-i;
  pthread_attr_setschedparam(&att[i], &parameter[i]);
}

if(pthread_create(&receive_thread, (void*)&att[0], receiver_function, NULL)==0)
	printf("\n\rReceiver Thread is created Sucessfully!\n\r");
  else perror("thread creation has failed");
  
  if(pthread_create(&send_thread, (void*)&att[1], sender_function, NULL)==0)
	printf("\n\rSender Thread  is Created Sucessfully!\n\r");
  else perror("thread creation has failed");

  pthread_join(receive_thread, NULL);
  pthread_join(send_thread, NULL);


  
}


