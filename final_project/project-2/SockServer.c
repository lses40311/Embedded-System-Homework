// Example of a Server

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "SockOp.h"

#define BUFSIZE 50
#define WAIT_TIME 100000

typedef struct thread_obj{
	int connfd;
} THREAD_OBJ;

void* ClientHandler(void *thread_in);

int Continue_flag = 1;


int main(int argc, char* argv[]){
/* Initial Socket */

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
	
	return 0;
	
}

void* ClientHandler(void *thread_in){
	pthread_detach(pthread_self()); 
	
	THREAD_OBJ* thr_obj = (THREAD_OBJ*) thread_in;
	int connfd = thr_obj->connfd;
	free(thr_obj);
	
	int n;
	char snd[BUFSIZE], rcv[BUFSIZE];

/* Step1 Handshake ID */
	// read message from client
	if ((n = recv(connfd, rcv, BUFSIZE, 0)) < 0)
		errexit("Error: read ID\n");
	

	// write message back to the client
	n = sprintf(snd, "%.*s", n, rcv);

	if ((n = send(connfd, snd, n, 0)) < 0)
		errexit("Error: write back ID\n");
	
/* Step2 I/O Control */
	char buf[BUFSIZE];

	/*
	fd_set read_fds, write_fds, except_fds;
	FD_ZERO(&read_fds);
	FD_ZERO(&write_fds);
	FD_ZERO(&except_fds);
	FD_SET(connfd, &read_fds);
	*/
	
	while(Continue_flag){
		n = send(connfd, buf, strlen(ID), 0);
		if (n <= 0){
			Continue_flag = 0;
			cout << "Disconnected\n" << endl;
		}
		usleep(WAIT_TIME);
		n = recv(connfd, buf, BUFSIZE, 0);
		if (n <= 0){
			Continue_flag = 0;
			cout << "Disconnected\n" << endl;
			break;
		}
		usleep(WAIT_TIME);
		
		
		/*
		struct timeval timeout;
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;
		
		if (select(connfd + 1, &read_fds, &write_fds, &except_fds, &timeout) >= 1){
			if (FD_ISSET(connfd, &read_fds)){
				memset(buf, 0, BUFSIZE);
				if ((n = recv(connfd, buf, BUFSIZE, 0)) == -1)
					errexit("Error: read()\n");
				else if (n == 0){
					printf("Client: %d Disconnected\n", connfd);
					break;
				}
				else
					printf("Client: %s %d\n", buf, n);
			}	
		}
		else{
			// Timeout
		}
		
		n = sprintf(buf, "Hello from Server");
		printf("%s  %d\n", buf, n);
		if ((n = send(connfd, buf, n, 0)) == -1)
			errexit("Error: write()\n");
		
		printf("? %s  %d \n", buf, n);
		
		*/
		
		
	}

	
	
	close(connfd);
	
	pthread_exit(NULL);
}
