#include <stdio.h>
#include <stdint.h> 
#include <stdlib.h>
#include <pthread.h>

void* addition_1(void* output_1)
{
 uint16_t i= 0;
 *((uint16_t*)output_1) = 0;
 for(i=1; i<100; i++)
 {
   *((uint16_t*)output_1)+= i;
 }
  printf("%d\n", *((uint16_t*)output_1));
 return output_1;
}


 void* addition_2(void* output_2)
{
 uint16_t j= 0;
 *((uint16_t*)output_2) = 0;
 for(j=100; j<200; j++)
 {
   *((uint16_t*)output_2)+= j;
 }
 printf("%d\n", *((uint16_t*)output_2));
 return output_2;
  
}

 void* addition_3(void* output_3)
{
 uint16_t k= 0;
 *((uint16_t*)output_3) = 0;
 for(k=200; k<300; k++)
 {
   *((uint16_t*)output_3)+= k;
 }
  printf("%d\n", *((uint16_t*)output_3));
 return output_3;
}

void main()
{
 pthread_t thread_1, thread_2, thread_3;
 uint8_t ret1= 0, ret2 = 0, ret3 =0;
 uint16_t result_1=0, result_2=0, result_3=0;
 void* result_1_ptr=&result_1;
 void* result_2_ptr=&result_2;
 void* result_3_ptr=&result_3;
 uint32_t total = 0;
 ret1 = pthread_create(&thread_1, NULL, addition_1, result_1_ptr);
 ret2 = pthread_create(&thread_2, NULL, addition_2, result_2_ptr);
 ret3 = pthread_create(&thread_3, NULL, addition_3, result_3_ptr);
 pthread_join(thread_1, &result_1_ptr);
 pthread_join(thread_2, &result_2_ptr);
 pthread_join(thread_3, &result_3_ptr);
 total = (*((uint16_t*) result_1_ptr) + *((uint16_t*) result_2_ptr) + *((uint16_t*) result_3_ptr));
 printf("%d is the sum of numbers from 1 to 299\n", total);
}

 
  
