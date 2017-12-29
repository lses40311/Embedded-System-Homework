/* 
 * project_func.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

void addOrder(int order_index, char order_name[]){

	switch(order_index){

		case 1:
			sprintf(order_name, "Subway");
			break;
		case 2:
			sprintf(order_name, "Coffee");
			break;
		case 3:
			sprintf(order_name, "Sandwich");
			break;
		default:
			break;

	}
}


void *threadMobileCounter(void *threadid)
{

	long tid;
	tid = (long)threadid;
	printf("tid => %ld\n", tid);

	pthread_exit(NULL);
	
}

void *threadWaitingList(void *threadid)
{

	long tid;
	tid = (long)threadid;
	printf("tid => %ld\n", tid);
	
	char order_name[20];
	addOrder(3, order_name);
	printf("order name: %s\n", order_name);

	pthread_exit(NULL);
	
}

void *threadPOS(void *threadid)
{
	long tid;
	tid = (long)threadid;
	printf("tid => %ld\n", tid);
	
	pthread_exit(NULL);
	
}
























