// Example of a client

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <iostream>
#include <string>

#include <pthread.h>

#include "SockOp.h"

using namespace std;

#define BUFSIZE 1000
#define WAIT_TIME 100000


string ID;

int Continue_flag = 1;


int connfd;		// socket descriptor

pthread_mutex_t vec_send_lock;
struct Node* vec_send_str;

pthread_mutex_t vec_recv_lock;
string show_str;


struct Data {
	char data0;
	char* data_arr;
};

struct Node {
	int priority;
	struct Data data;
	struct Node *next;
};

void enqueue(struct Node** root, int priority, struct Data data);
struct Node* dequeue(struct Node** root);
int GetQueueLength(struct Node* root);

void* KeyInHandler(void *thread_in);
void* OutputHandler(void *thread_in);


int main(int argc, char* argv[]){
	


	
	
/* Socket */

	int n;
	char buf[BUFSIZE];
	char rcv_buf[BUFSIZE];
	
	if (argc < 3)
		errexit("Usage: %s host_address host_port\n", argv[0]);
	
	// Create socket and connect to server
	connfd = connectsock(argv[1], argv[2], "tcp");
	
	
	cout << "===========  PXA Simulator  ============" << endl;
	cout << "1. Mobile Counter" << endl;
	cout << "2. Waiting List" << endl;
	cout << "3. POS" << endl;
	cout << "Please choose which you want to simulate (1~3): ";
	
	int type_idx;
	cin >> type_idx;
	
	if (type_idx == 1)
		ID = "MC\0";
	else if (type_idx == 2)
		ID = "WL\0";
	else if (type_idx == 3)
		ID = "POS\0";
	
	
/* Start Client Task */



	pthread_t key_thread;
	int rt_val;
	rt_val = pthread_create(&key_thread, NULL, KeyInHandler, NULL);
	if (rt_val){
		errexit("Error while opening the thread");
	}
	pthread_t lcd_thread;
	rt_val = pthread_create(&lcd_thread, NULL, OutputHandler, NULL);
	if (rt_val){
		errexit("Error while opening the thread");
	}
	

/* Step1 Handshake ID */
	if ((n = write(connfd, ID.c_str(), ID.length())) == -1)
		errexit("Error: Write ID\n");
	printf("Send ID: %d\n", n);
	
	// read message from the server and print
	memset(buf, 0, BUFSIZE);
	if ((n = read(connfd, buf, BUFSIZE)) == -1){
		errexit("Error: read()\n");
	}
	
	buf[n] = '\0';
	if (ID.compare(buf) != 0){
		errexit("Handshake ID fails with id: %s\n", buf);
	}
	
	
	cout << endl << "==== Start ==== (Press key in as input)" << endl;
	
	
/* Step2 I/O Control */

	
	while(Continue_flag){
		memset(buf, 0, BUFSIZE);
		
		
		usleep(WAIT_TIME);
	
		pthread_mutex_lock(&vec_send_lock);
		if (GetQueueLength(vec_send_str) == 0){
			buf[0] = 'e';
			buf[1] = '\0';
		}
		else{
			int i;
			for (i = 0; i < GetQueueLength(vec_send_str); i++){
				struct Node* tmp_node = dequeue(&vec_send_str);
				buf[i] = (tmp_node->data).data0;
			}
			buf[i] = '\0';
		}
		pthread_mutex_unlock(&vec_send_lock);
		
		n = write(connfd, buf, strlen(buf));
		if (n <= 0){
			Continue_flag = 0;
			printf("%d write Disconnected\n", n);
			break;
		}
		
		
		usleep(WAIT_TIME);
		
			
		memset(rcv_buf, 0, BUFSIZE);
		n = read(connfd, rcv_buf, BUFSIZE);
		if (n <= 0){
			Continue_flag = 0;
			printf("%d recv Disconnected\n", n);
			break;
		}	
		pthread_mutex_lock(&vec_recv_lock);
		show_str = rcv_buf;
		pthread_mutex_unlock(&vec_recv_lock);
	}

	
	
	//close client socket
	close(connfd);
	
	return 0;
}

void* KeyInHandler(void *thread_in){
	while (Continue_flag){
		
		char in_char;
		
		cin >> in_char;
		
		struct Data tmp_data;
		tmp_data.data0 = in_char;
		pthread_mutex_lock(&vec_send_lock);
		enqueue(&vec_send_str, 0, tmp_data);
		pthread_mutex_unlock(&vec_send_lock);
	}
	
}
	
void* OutputHandler(void *thread_in){
	string pre_str = show_str;
	while (Continue_flag){
		// Prepare output string to display data structure

		pthread_mutex_lock(&vec_recv_lock);
		if (pre_str.compare(show_str) != 0){
			if (show_str.compare("e") != 0)
				cout << "\noutput:\n" << show_str << endl << endl;
			pre_str = show_str;
		}
		pthread_mutex_unlock(&vec_recv_lock);
		
		usleep(WAIT_TIME);
	}
}


void enqueue(struct Node** root, int priority, struct Data data){
	struct Node *tmp, *ptr;
	
	// Initial tmp node
	tmp = (struct Node *)malloc(sizeof(struct Node));
	tmp->data = data;
	tmp->priority = priority;
	tmp->next = NULL;
	
	//Insert the element to the list
	if ((*root) == NULL)
		(*root) = tmp;
	else if ((*root)->priority > priority){
		// Swap root if neccesary
		tmp->next = (*root);
		(*root) = tmp;
	}
	else{
		// Insert to the list if neccesary
		ptr = (*root);
		while (ptr->next != NULL && (ptr->next)->priority <= priority){
			ptr = ptr->next;
		}
		tmp->next = ptr->next;
		ptr->next = tmp;
	}
}

struct Node* dequeue(struct Node** root){
	struct Node* return_data;
	
	if ((*root) != NULL){
		return_data = (*root);
		(*root) = (*root)->next;
		return return_data;
	}
	else{
		return NULL;
	}
	
}


int GetQueueLength(struct Node* root){
	int counter = 0;
	struct Node* ptr = root;
	
	while (ptr != NULL){
		counter++;
		ptr = ptr->next;
	}
	
	return counter;
}
