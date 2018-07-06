/************************
* Watchdog timer
* Author: Sanika
* Date: 07/05/18
************************/

#include<stdio.h>
#include<stdint.h>
#include<time.h>
#include<stdlib.h>
#include<pthread.h>

pthread_mutex_t mutex;	// Mutex declaration
struct timespec tenseconds;
//Structure that contains various members: x,y,z,acc,yaw,pitch,roll,count and another structure timespec
typedef struct {
float x;
float y;
float z;
float acc;
float yaw;
float pitch;
float roll;
float count;
struct timespec presctimestamp;
}time_struct;

time_struct attitude_precision;
 
void *update_members(void *members)//Thread1: Update thread 
{ 
pthread_mutex_lock(&mutex); //Locking the Mutex
int val = 1;
val++;
printf("Update values function\n");
attitude_precision.count++;
attitude_precision.x+=val;
attitude_precision.y+=val;
attitude_precision.z+=val;
attitude_precision.acc+=val;
attitude_precision.yaw+=val;
attitude_precision.pitch+=val;
attitude_precision.roll+=val;
clock_gettime(CLOCK_REALTIME,&(attitude_precision.presctimestamp));
printf("x value is %f\n",attitude_precision.x);
printf("y value is %f\n",attitude_precision.y);
printf("z value is %f\n",attitude_precision.z);
printf("acceleration value is %f\n",attitude_precision.acc);
printf("yaw value is %f\n",attitude_precision.yaw);
printf("pitch value is %f\n",attitude_precision.pitch);
printf("roll value is %f\n",attitude_precision.roll);
printf("Timestamp = %ld\n",attitude_precision.presctimestamp.tv_nsec);
usleep(12000000);

pthread_mutex_unlock(&mutex);


}

void *read_members(void *members)

{
int val=-1;
while(val!=0){
tenseconds.tv_sec=10;
val=pthread_mutex_timedlock(&mutex,&tenseconds);	// Mutex timed lock
printf("No new data available at %ld\n",attitude_precision.presctimestamp.tv_sec);
}

//pthread_mutex_lock(&mutex);

printf("Reading the updated values\n");
printf("x value is %f\n",attitude_precision.x);
printf("y value is %f\n",attitude_precision.y);
printf("z value is %f\n",attitude_precision.z);
printf("acceleration value is %f\n",attitude_precision.acc);
printf("yaw value is %f\n",attitude_precision.yaw);
printf("pitch value is %f\n",attitude_precision.pitch);
printf("roll value is %f\n",attitude_precision.roll);
printf("Timestamp = %ld\n",attitude_precision.presctimestamp.tv_nsec);

pthread_mutex_unlock(&mutex);	// Mutex unlock


}


void main(){

pthread_mutex_init(&mutex,NULL);	// Initialising mutex
pthread_t write[2],read[2],update10;
int val=0;
for(val=0;val<2;val++){
pthread_create(&read[val],NULL,&read_members,NULL);	// Creating read thread
pthread_create(&write[val],NULL,&update_members,val);		// Creating write thread
//pthread_create(&update10,NULL,&updateevery10,NULL);

pthread_join(write[val],NULL);		// Join write thread
pthread_join(read[val],NULL);			// Join read thread
//pthread_join(updateevery10,NULL);
}


}
