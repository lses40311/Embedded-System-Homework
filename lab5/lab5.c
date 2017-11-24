#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/sem.h>

#define PXA 0
#define newUBER 0
#define MAX_AXIS_X 9
#define MAX_AXIS_Y 9
#define MY_KEY 5566

#if PXA
#include "asm-arm/arch-pxa/lib/creator_pxa270_lcd.h"
#endif

// Link list structure
typedef struct link_list{
    int data[4] ;
    int priority ;
    struct  link_list * next ;
} link_list ;
//Link list structure

// Function Declare
void * uber_thread() ;
void * customer_thread() ;
void * driving_thread() ;
void enqueue_safe() ;
void show_queue() ;
char * get_path_str() ;
link_list * enqueue() ;
link_list * dequeue() ;
int my_sem_init() ;
int my_sem_remove() ;
int my_sem_post() ;
int my_sem_wait() ;

// Global Variables
link_list * head_request = NULL ;
link_list * head_uber = NULL ;
pthread_mutex_t lock, navigate_sig;
int * input_data ;
link_list * paired_data ;
int lcd_fd ;
int request_sem, uber_sem ; // originally sem_t


// Handles Ctrl + c
void intHandler(int dummy) {
    my_sem_remove(request_sem, 1) ;
    my_sem_remove(uber_sem, 1) ;
    exit(0) ;
}


int main(int argc, char ** argv){
    if(argc != 2){
        printf("Wrong Parameters\nUsage: ./a uberfile\n");
        exit(EXIT_FAILURE) ;
    }
    const char * uber_loc_file = argv[1] ;
    paired_data = malloc(2 * sizeof(link_list));

    // remove semaphore if system interupted
    signal(SIGINT, intHandler);

    // Initialize Semaphore & mutex
    //sem_init(&request_sem, 0, 0); // another way to open a semaphore
    if((request_sem = my_sem_init(MY_KEY, 1, 0)) < 0){
        printf("%s\n", "open sem error") ;
        exit(1) ;
    }
    pthread_mutex_init(&lock, NULL) ;
    pthread_mutex_init(&navigate_sig, NULL) ;
    pthread_mutex_lock(&navigate_sig) ;

    #if PXA
    // Initialize PXA I/O
	unsigned short key ;
    if((lcd_fd = open("/dev/lcd", O_RDWR)) < 0){
        printf("open failed\n") ;
        exit(-1) ;
    }
    ioctl(lcd_fd, KEY_IOCTL_CLEAR, key) ;
    ioctl(lcd_fd, LCD_IOCTL_CLEAR, NULL) ;
    printf("LCD Initialization Success.\n") ;
    #endif

    //create two worker threads
    pthread_t* thread_handles = malloc(3 * sizeof(pthread_t));
    pthread_create(&thread_handles[0], NULL, customer_thread, NULL);
    pthread_create(&thread_handles[1], NULL, uber_thread, (void*) uber_loc_file );

    // Create a navigation thread
    while(1){
        pthread_mutex_lock(&navigate_sig) ;
        printf("Create driving thread\n") ;
        pthread_create(&thread_handles[3], NULL, driving_thread, NULL);
        pthread_detach(thread_handles[3]) ;
        //pthread_mutex_unlock(&navigate_sig) ;
    }

    // Prevent the main thread from terminated.
    sleep(300) ;

    return 0 ;
}

void * customer_thread(){
    int * requests = malloc(4*sizeof(int)) ;
    unsigned short key = 0 ;
    int priority = 0 ;
    while (1) {
        printf("Please Enter 4 numbers as the customer's request..\n") ;
        int i = 0;
        for(i =0; i<4; i++){
            #if PXA
            ioctl(lcd_fd, KEY_IOCTL_WAIT_CHAR, &key) ;
            key = ((key&0xff)-48) ;
            #else
            scanf("%hu", &key) ;
            #endif
            requests[i] = (int)key ;
			//printf("==%hu\n", key&0xff) ;
        	printf("read %d\n", requests[i]) ;
		}
        enqueue_safe(&head_request, requests, priority);
        my_sem_post(request_sem, 0);
    }

}

void * uber_thread(void * arg){
    char *val_p = (char *) arg ;
    unsigned int uber_cnt = 0 ;
    int * coordinates = malloc(4*sizeof(int));
    int i ;
    for(i=0; i<4; i++) coordinates[i] = 0 ;
    FILE * fp = fopen(val_p, "r") ;
    while(fscanf(fp, "%d %d\n", &coordinates[0], &coordinates[1]) > 0){
        printf("val=%d,%d\n", coordinates[0], coordinates[1]) ;
        head_uber = enqueue(head_uber, coordinates, 0);
		uber_cnt++ ;
	}

    // Initialize Semaphore with uber number
    //sem_init(&uber_sem, 0, uber_cnt) ;
    if((uber_sem = my_sem_init( MY_KEY+1, 1, uber_cnt)) < 0){
        printf("%s\n", "open uber_sem error") ;
        exit(1) ;
    }

    while (1) {
        //sem_wait(&request_sem) ;
        //sem_wait(&uber_sem) ;
        my_sem_wait(request_sem, 0);
        my_sem_wait(uber_sem, 0);
        pthread_mutex_lock(&lock);
        printf("%s\n", "Find a car.");
        head_uber = dequeue(head_uber, 0) ;
        head_request = dequeue(head_request, 1) ;
        pthread_mutex_unlock(&lock);
        pthread_mutex_unlock(&navigate_sig) ; // Start Driving Thread
        //pthread_mutex_lock(&navigate_sig) ;
    }
}

void * driving_thread(){
    link_list uber, request ;
    uber = paired_data[0] ;
    request = paired_data[1] ;
    printf("%s\n", "---------------------");
    char * str1 = get_path_str(uber.data[0], uber.data[1], request.data[0], request.data[1]);
    printf("%s\n", str1 );
    char * str2 = get_path_str(request.data[0], request.data[1], request.data[2], request.data[3]) ;
    printf("%s\n", str2 );
    printf("%s\n", "---------------------");

    #if PXA
	ioctl(lcd_fd, LCD_IOCTL_CLEAR, NULL) ;
    lcd_write_info_t display ;
    display.Count = sprintf((char*) display.Msg, str1) ;
    ioctl(lcd_fd, LCD_IOCTL_WRITE, &display) ;
	display.Count = sprintf((char*) display.Msg, str2) ;
	ioctl(lcd_fd, LCD_IOCTL_WRITE, &display) ;
    #endif

    int * uber_return = malloc(4*sizeof(int)) ;
    uber_return[0] = request.data[2] ;
    uber_return[1] = request.data[3] ;
    uber_return[2] = 0 ;
    uber_return[3] = 0 ;
    enqueue_safe(&head_uber, uber_return, 0) ;
    //sem_post(&uber_sem) ;
    my_sem_post(uber_sem, 0) ;

    free(str1) ;
	free(str2) ;
    free(uber_return) ;
    pthread_exit(NULL);
}

link_list * dequeue(link_list * head, int idx){
	if(head != NULL){
        show_queue(head) ;
        paired_data[idx] = *head ;
		return head->next ;
	}
	else{
		printf("Queue is empty.\n") ;
		return head ;
	}
}

void enqueue_safe(link_list ** head, int * vals, int priority){
    pthread_mutex_lock(&lock);
    *head = enqueue(*head, vals, priority) ;
    pthread_mutex_unlock(&lock);
}

link_list * enqueue(link_list * head, int * vals, int priority){
	link_list * node = (link_list*) malloc(sizeof(link_list)) ;
    int i ;
    for (i = 0; i < 4; i++) {
        node->data[i] = vals[i] ;
    }
	node->priority = priority ;
	node->next = NULL ;
	if(head == NULL){
		return node ;
	}
	else{
		if(priority < head->priority){
			node->next = head ;
			return node ;
		}
		link_list * ptr = head ;
		while(ptr->next != NULL){
			// Check priority
			if(priority < (ptr->next)->priority){
				node->next = ptr->next ;
				ptr->next = node ;
				return head ;
			}
			ptr = ptr->next ;
		}
		// Next node is NULL
		ptr->next = node ;
	}
	return head ;
}

char * get_path_str(int src_x, int src_y, int dest_x, int dest_y){
    char * str = malloc(50*sizeof(char)) ;
    int len = sprintf(str, "%d,%d->%d,%d==>", src_x, src_y, dest_x, dest_y) ;
    sprintf(str+len, "%d,%d->%d,%d->%d,%d\n",src_x, src_y, src_x, dest_y, dest_x, dest_y) ;
    return str ;
}

void show_queue(link_list * head){
	link_list * ptr = head ;
    int i ;
	while(ptr != NULL){
        for (i = 0; i < 4; i++) {
            printf("%d ", ptr->data[i]) ;
        }
		printf(" -> ");
		ptr = ptr->next ;
	}
    printf("\n");
}

int my_sem_init(long int key, int sem_num, int init_val){
    int s, i ;
    s = semget(key, sem_num, IPC_CREAT | IPC_EXCL | 0644) ;
    if(s < 0){
        printf("%s\n", "ERROR in getting sem");
        return s ;
    }
    for( i = 0; i < sem_num; i++){
        if(semctl(s, i, SETVAL, init_val) < 0){
            printf("%s\n", "ERROR in setting sem value");
        }
    }
    return s ;
}

int my_sem_remove(int s, int sem_num){
    int i ;
    for( i = 0; i < sem_num; i++){
        if(semctl(s, i, IPC_RMID, 0) < 0){
            printf("%s\n", "ERROR in removing sem");
            return -1 ;
        }
    }
    return 0 ;
}

int my_sem_post(int s, int sem_idx){
    struct sembuf sop ;
    sop.sem_num = sem_idx ;
    sop.sem_op = 1 ;
    sop.sem_flg = 0 ;
    if(semop(s, &sop, 1) < 0){
        printf("%s\n", "ERROR posting sem");
        return -1 ;
    }
    else{
        return 0;
    }
}

int my_sem_wait(int s, int sem_idx){
    struct sembuf sop ;
    sop.sem_num = sem_idx ;
    sop.sem_op = -1 ;
    sop.sem_flg = 0 ;
    if(semop(s, &sop, 1) < 0){
        printf("%s\n", "ERROR waiting sem");
        return -1 ;
    }
    else{
        return 0;
    }
}
