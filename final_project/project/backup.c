/* 
 * project.c
 */
 

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

void *threadUberQueue(void *threadid)
{

	long tid;
	tid = (long)threadid;
	
	pthread_exit(NULL);
	
}

void *threadCustomerRequest(void *threadid)
{

	long tid;
	tid = (long)threadid;
	
	pthread_exit(NULL);
	
}

void *threadUberDriving(void *threadid)
{
	long tid;
	tid = (long)threadid;
	
	pthread_exit(NULL);
	
}


int main(int argc, char *argv[])
{

	
	pthread_t thread_MobileCounter, thread_WaitingList, thread_POS;
	
	int rc, tid;
	
	tid = 1;
	rc = pthread_create(&thread_MobileCounter, NULL, threadMobileCounter, (void *)tid);	
	if(rc){
		printf("ERROR: return code from pthread create() is %d\n", rc);
		exit(-1);	
	}

	tid = 2;
	rc = pthread_create(&thread_WaitingList, NULL, threadWaitingList, (void *)tid);	
	if(rc){
		printf("ERROR: return code from pthread create() is %d\n", rc);
		exit(-1);	
	}
		
	tid = 3;	
	rc = pthread_create(&thread_POS, NULL, threadPOS, (void *)tid);	
	if(rc){
		printf("ERROR: return code from pthread create() is %d\n", rc);
		exit(-1);	
	}

	while(1);
	
	printf("Main: program completed. Exiting.\n");
	pthread_exit(NULL);

	
}

