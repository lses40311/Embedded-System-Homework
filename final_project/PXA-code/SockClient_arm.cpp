// Example of a client

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>

#include <pthread.h>

#include "SockOp.h"
#include "creator_pxa270_lcd.h"


#define BUFSIZE 50
#define WAIT_TIME 100000


char ID[BUFSIZE] = "MC1\0";

int Continue_flag = 1;

int LCD_IO_fd = 0;
pthread_mutex_t lcd_ctl_lock;


int connfd;		// socket descriptor

pthread_mutex_t vec_send_lock;
queue<string> vec_send_str;
pthread_mutex_t vec_recv_lock;
queue<string> vec_recv_str;


struct Data {
	int id;
	
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
	
	
/* PXA270 I/O Control */
	
	LCD_IO_fd = open("/dev/lcd", O_RDWR);		// O_RDWR: Open for reading and writing
	if (LCD_IO_fd < 0){
		printf("Fail to open dev!\n");
		return -1;
	}
	// LCD
	ioctl(LCD_IO_fd, LCD_IOCTL_CLEAR, NULL);
	
	// Keypad
	unsigned short dump_ky;
	ioctl(LCD_IO_fd, KEY_IOCTL_CLEAR, dump_ky);
	
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

	
	
/* Socket */

	int n;
	char buf[BUFSIZE];
	
	if (argc < 3)
		errexit("Usage: %s host_address host_port\n", argv[0]);
	
	// Create socket and connect to server
	connfd = connectsock(argv[1], argv[2], "tcp");
	
	
/* Start Client Task */


/* Step1 Handshake ID */
	if ((n = write(connfd, ID, strlen(ID))) == -1)
		errexit("Error: Write ID\n");
	printf("Send ID: %d\n", n);
	
	// read message from the server and print
	memset(buf, 0, BUFSIZE);
	if ((n = read(connfd, buf, BUFSIZE)) == -1){
		errexit("Error: read()\n");
	}
	
	buf[n] = '\0';
	if (strcmp(ID, buf) != 0){
		errexit("Handshake ID fails with id: %s\n", buf);
	}
	
	
/* Step2 I/O Control */

	/*	Asynchronize
	fd_set read_fds, write_fds, except_fds;
	FD_ZERO(&read_fds);
	FD_ZERO(&write_fds);
	FD_ZERO(&except_fds);
	FD_SET(connfd, &read_fds);
	*/
	
	while(Continue_flag){
		
	//	memset(buf, 0, BUFSIZE);
		
		
	//	usleep(WAIT_TIME);
	
		string send_str = "";
		pthread_mutex_lock(&vec_send_lock);
		if (vec_send_str.size() == 0){
			send_str = "e";
		}
		else{
			for (int i = 0; i < vec_send_str.size(); i++){
				send_str += vec_send_str.front();
				vec_send_str.pop();
			}
		}
		pthread_mutex_unlock(&vec_send_lock);
		
		n = write(connfd, send_str.c_str(), send_str.length());
		if (n <= 0){
			Continue_flag = 0;
			printf("%d write Disconnected\n", n);
			break;
		}
		
		
	//	usleep(WAIT_TIME);
		
		
		
		n = read(connfd, buf, BUFSIZE);
		if (n <= 0){
			Continue_flag = 0;
			printf("%d recv Disconnected\n", n);
			break;
		}
		
		string recv_str(buf);
		pthread_mutex_lock(&vec_recv_lock);
			vec_recv_str.push(recv_str);
		pthread_mutex_unlock(&vec_recv_lock);
		
		
		
		
		
		
		/*	Asynchronize
		struct timeval timeout;
		timeout.tv_sec = 3;
		timeout.tv_usec = 0;
		
		if (select(connfd + 1, &read_fds, &write_fds, &except_fds, &timeout) >= 1){
			
			if (FD_ISSET(connfd, &read_fds)){
				memset(buf, 0, BUFSIZE);
				if ((n = recv(connfd, buf, BUFSIZE, 0)) == -1)
					errexit("Error: read()\n");
				else if (n == 0){
					printf("Server: %d Disconnected\n", connfd);
					Continue_flag = 0;
					break;
				}
				else
					printf("Server: %s\n", buf);
			}
		}
		else{
			
			// Timeout
		}
		
		*/
	}

	
	
	//close client socket
	close(connfd);
	close(LCD_IO_fd);
	
	return 0;
}

void* KeyInHandler(void *thread_in){
	unsigned short key;
	int ret;
	
	int rt_LCD;
	
	while (Continue_flag){
		
		pthread_mutex_lock(&lcd_ctl_lock);
		rt_LCD = ioctl(LCD_IO_fd, KEY_IOCTL_CHECK_EMTPY, &key);
		pthread_mutex_unlock(&lcd_ctl_lock);
		
		while (rt_LCD < 0){
			pthread_mutex_lock(&lcd_ctl_lock);
			rt_LCD = ioctl(LCD_IO_fd, KEY_IOCTL_CHECK_EMTPY, &key);
			pthread_mutex_unlock(&lcd_ctl_lock);
			sleep(1);
		}
		
		ret = ioctl(LCD_IO_fd, KEY_IOCTL_GET_CHAR, &key);
		
		cout << "KeyIn: " << (char)(key&0xff) << endl;
		string key_str = "";
		key_str += (char)(key&0xff);
		
		pthread_mutex_lock(&vec_send_lock);
		vec_send_str.push(key_str);
		pthread_mutex_unlock(&vec_send_lock);
	}
	
}
void* OutputHandler(void *thread_in){
	lcd_write_info_t display;
	
	// Clear LCD
	pthread_mutex_lock(&lcd_ctl_lock);
	ioctl(LCD_IO_fd, LCD_IOCTL_CLEAR, NULL);
	pthread_mutex_unlock(&lcd_ctl_lock);
	
	// Prepare output string to display data structure
	string print_str = "";
	pthread_mutex_lock(&vec_recv_lock);
		for (int i = 0; i < vec_recv_str.size()-1; i++){
			vec_recv_str.pop();
		}
		print_str = vec_recv_str.front();
		vec_recv_str.pop();
	pthread_mutex_unlock(&vec_recv_lock);
	
	display.Count = sprintf((char*) display.Msg, "Hello");
//	display.Count = sprintf((char*) display.Msg, print_str.c_str());
	
	// Print out to LCD
	pthread_mutex_lock(&lcd_ctl_lock);
	ioctl(LCD_IO_fd, LCD_IOCTL_WRITE, &display);
	pthread_mutex_unlock(&lcd_ctl_lock);
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
