/************
*Author: Sanika
* Date: 07/05/18
****************/
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <pthread.h>

pthread_mutex_t mutex; //Mutex declaration

typedef struct 
{                     //Structure with members: x,y,z,acceleration,yaw,pitch,count and another structure timespec

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

void* update_members(void* members) //Thread1: Update thread
{

	pthread_mutex_lock(&mutex); //Locking the Mutex
	int val =1;
        printf("Update values function\n");
        val++;
        attitude_precision.count++;
        attitude_precision.x += val;
        attitude_precision.y += val;
	attitude_precision.z += val;
	attitude_precision.acc += val;
	attitude_precision.yaw += val;
	attitude_precision.pitch += val;
	attitude_precision.roll += val;
	clock_gettime(CLOCK_REALTIME, &(attitude_precision.presctimestamp)); //Using clock_getime() function for getting prescise time
	printf("x value is %f\n", attitude_precision.x);
	printf("y value is %f\n", attitude_precision.y);
	printf("z value is %f\n", attitude_precision.z);
	printf("acceleration value is %f\n", attitude_precision.acc);
	printf("yaw value is %f\n", attitude_precision.yaw);
	printf("pitch value is %f\n", attitude_precision.pitch);
	printf("roll value is %f\n", attitude_precision.roll);
	printf("Current Timestamp is %ld\n", attitude_precision.presctimestamp.tv_sec); // For printing current time in terms of seconds unit
	printf("Current Timestamp is %ld\n", attitude_precision.presctimestamp.tv_nsec); // For printing current time in terms of nanoseconds unit
	pthread_mutex_unlock(&mutex); //Unlocking the Mutex
}

void* read_members(void* members) //Thread2: Read the values

{
	pthread_mutex_lock(&mutex); //Locking the Mutex
	printf("Read values function\n");
	printf("x value is %f\n", attitude_precision.x);
	printf("y value is %f\n", attitude_precision.y);
	printf("z value is %f\n", attitude_precision.z);
	printf("acc value is %f\n", attitude_precision.acc);
	printf("yaw value is %f\n", attitude_precision.yaw);
	printf("pitch value is %f\n", attitude_precision.pitch);
	printf("roll value is %f\n", attitude_precision.roll);
	printf("Current Timestamp is %ld\n", attitude_precision.presctimestamp.tv_sec); // For printing current time in terms of seconds unit
	printf("Current Timestamp is %ld\n", attitude_precision.presctimestamp.tv_nsec); // For printing current time in terms of nanoseconds unit
	pthread_mutex_unlock(&mutex); //Unlocking the Mutex
}

void main()

{
	pthread_mutex_init(&mutex, NULL); //Mutex initialization
	pthread_t write[2], read[2];
	int val =0;
	for(val=0; val<2; val++)
	{


	pthread_create(&read[val], NULL, &read_members, NULL); //pthread 2 create
	pthread_create(&write[val], NULL, &update_members, NULL); //pthread 1 create
	pthread_join(write[val], NULL); //pthread 1 join
        pthread_join(read[val], NULL); //pthread 2 join
         }
}
	
	

	
