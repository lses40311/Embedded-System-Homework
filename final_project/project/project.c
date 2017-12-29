/* 
 * project.c
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "project_header.h"


int main(int argc, char *argv[])
{

	
	pthread_t thread_MobileCounter, thread_WaitingList, thread_POS;
	void *n1, *n2, *n3;
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

	pthread_join(thread_MobileCounter,&n1); 
	pthread_join(thread_WaitingList,&n2); 
	pthread_join(thread_POS,&n3); 
	
	printf("Main: program completed. Exiting.\n");
	pthread_exit(NULL);
	return(0);
	
}

