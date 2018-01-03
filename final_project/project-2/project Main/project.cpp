/* 
 * project.c
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <iostream>
#include <queue>


#include "POS.hpp"
#include "WL.hpp"
#include "MC.hpp"
#include "Sock.hpp"
#include "project_header.h"
#include "global.hpp"

using namespace std;

/***** Declare global variables ****/

pthread_mutex_t MC_recv_List_lock;
pthread_mutex_t MC_send_List_lock;
queue<char> MC_recv_List;
string MC_send_List;

pthread_mutex_t WL_recv_List_lock;
pthread_mutex_t WL_send_List_lock;
queue<char> WL_recv_List;
string WL_send_List;

pthread_mutex_t POS_recv_List_lock;
pthread_mutex_t POS_send_List_lock;
queue<char> POS_recv_List;
string POS_send_List;

pthread_mutex_t Order_List_lock;
queue<char> Order_recv_List;

/***** Declare global variables ****/


int main(int argc, char *argv[])
{

	pthread_t thread_MobileCounter, thread_WaitingList, thread_POS;
	int rc, tid;

	
/*****************  Services  ********************/
	tid = 1;
	rc = pthread_create(&thread_MobileCounter, NULL, threadMobileCounter, NULL);	
	if(rc){
		printf("ERROR: return code from pthread create() is %d\n", rc);
		exit(-1);	
	}

	tid = 2;
	rc = pthread_create(&thread_WaitingList, NULL, threadWaitingList, NULL);	
	if(rc){
		printf("ERROR: return code from pthread create() is %d\n", rc);
		exit(-1);	
	}
		
	tid = 3;	
	rc = pthread_create(&thread_POS, NULL, threadPOS, NULL);	
	if(rc){
		printf("ERROR: return code from pthread create() is %d\n", rc);
		exit(-1);	
	}
/*****************  Services  ********************/

	
	
/*****************  PXA Socket (Endless loop) *****************/
	int sockfd, connfd;		// socket descriptor
	struct sockaddr_in addr_cln;
	socklen_t sLen = sizeof(addr_cln);
	
	if (argc != 2)
		errexit("Usage: %s port\n", argv[0]);
		
	// Create socket and bind socket to port
	sockfd = passivesock(argv[1], "tcp", 10);

	while(1){
		// waiting for connection
		connfd = accept(sockfd, (struct sockaddr*)&addr_cln, &sLen);
		
		if (connfd == -1)
			errexit("Error: accept()\n");
		else{
			pthread_t cli_thread;
			
			THREAD_OBJ* thr_obj = (THREAD_OBJ *) malloc(sizeof(THREAD_OBJ));
			thr_obj->connfd = connfd;
			
			int rt_val;
			rt_val = pthread_create(&cli_thread, NULL, ClientHandler, (void*)thr_obj);
			if (rt_val){
				errexit("Error while opening the thread\n");
			}
		}
	}
	// Close server socket
	close(sockfd);
/*****************  PXA Socket *****************/
	
	
	printf("Main: program completed. Exiting.\n");
	pthread_exit(NULL);
	return(0);
	
}

