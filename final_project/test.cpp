/* 
 * project.c
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <iostream>
#include <queue>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>


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

	cout << "===========  PXA Simulator  ============" << endl;
	cout << "1. Mobile Counter" << endl;
	cout << "2. Waiting List" << endl;
	cout << "3. POS" << endl;
	cout << "Please choose which you want to simulate (1~3): ";
	
	int type_idx;
	cin >> type_idx;
	
	cout << endl << "==== Start ==== (Press key in as input)" << endl;
#pragma omp parallel
{
	#pragma omp single
	{
		#pragma omp task
		{
			while (1){
				char input;
				cin >> input;
				
				if (type_idx == 1){
					pthread_mutex_lock(&MC_recv_List_lock);
					MC_recv_List.push(input);
					pthread_mutex_unlock(&MC_recv_List_lock);
				}
				else if (type_idx == 2){
					pthread_mutex_lock(&WL_recv_List_lock);
					WL_recv_List.push(input);
					pthread_mutex_unlock(&WL_recv_List_lock);
				}
				else if (type_idx == 3){
					pthread_mutex_lock(&POS_recv_List_lock);
					POS_recv_List.push(input);
					pthread_mutex_unlock(&POS_recv_List_lock);
				}
			}
		}
		string pre_str = "";
		while (1){
			usleep(200000);
			if (type_idx == 1){
				pthread_mutex_lock(&MC_send_List_lock);
				if (MC_send_List.length() > 0 && MC_send_List.compare(pre_str) != 0){
					cout << "\noutput:\n" << MC_send_List << endl << endl;
					pre_str = MC_send_List;
				}
				pthread_mutex_unlock(&MC_send_List_lock);
			}
			else if (type_idx == 2){
				pthread_mutex_lock(&WL_send_List_lock);
				if (WL_send_List.length() > 0 && WL_send_List.compare(pre_str) != 0){
					cout << "\noutput:\n" << WL_send_List << endl << endl;
					pre_str = WL_send_List;
				}
				pthread_mutex_unlock(&WL_send_List_lock);
			}
			else if (type_idx == 3){
				pthread_mutex_lock(&POS_send_List_lock);
				if (POS_send_List.length() > 0 && POS_send_List.compare(pre_str) != 0){
					cout << "\noutput:\n" << POS_send_List << endl << endl;
					pre_str = POS_send_List;
				}
				pthread_mutex_unlock(&POS_send_List_lock);
			}
		}
	}
}
	
	
	printf("Main: program completed. Exiting.\n");
	pthread_exit(NULL);
	return(0);
	
}

