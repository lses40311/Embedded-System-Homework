#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define PXA 0
#define newUBER 0
#define MAX_AXIS_X 9
#define MAX_AXIS_Y 9

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
link_list * enqueue() ;
link_list * dequeue() ;

// Global Variables
link_list * head_request = NULL ;
link_list * head_uber = NULL ;
sem_t * request_sem, *uber_sem ;
pthread_mutex_t lock, navigate_sig;
int * input_data ;
link_list * paired_data ;

int main(int argc, char ** argv){
    if(argc != 2){
        printf("Wrong Parameters\nUsage: ./a uberfile\n");
        exit(EXIT_FAILURE) ;
    }
    const char * uber_loc_file = argv[1] ;
    paired_data = malloc(2 * sizeof(link_list));

    // Initialize Semaphore & mutex
    //sem_init(&request_sem, 0, 0); // another way to open a semaphore
    request_sem = sem_open("test",O_CREAT, 0644, 0) ;
    pthread_mutex_init(&lock, NULL) ;
    pthread_mutex_init(&navigate_sig, NULL) ;
    pthread_mutex_lock(&navigate_sig) ;

    #if PXA
    // Initialize PXA I/O
    int lcd_fd ;
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
        printf("Create navigation thread\n") ;
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
            #else
            scanf("%hu", &key) ;
            #endif
            printf("read: %hu\n", key) ;
            requests[i] = (int)key;
        }
        enqueue_safe(head_request, requests, priority);
        sem_post(request_sem);
    }

}

void * uber_thread(void * arg){
    char *val_p = (char *) arg ;
    int uber_cnt = 0 ;
    int * coordinates = malloc(4*sizeof(int));
    int i ;
    for(i=0; i<4; i++) coordinates[i] = 0 ;
    FILE * fp = fopen(val_p, "r") ;
    while(fscanf(fp, "%d %d\n", &coordinates[0], &coordinates[1]) > 0){
        printf("val=%d,%d\n", coordinates[0], coordinates[1]) ;
        head_uber = enqueue(head_uber, coordinates, 0);
		uber_cnt++ ;
        //show_queue(head) ;
	}

    // Initialize Semaphore with uber number
    uber_sem = sem_open("uber",O_CREAT, 0644, uber_cnt) ;

    while (1) {
        sem_wait(request_sem) ;
        sem_wait(uber_sem) ;

        pthread_mutex_lock(&lock);
        printf("%s\n", "---------------------");
        head_uber = dequeue(head_uber, 0) ;
        head_request = dequeue(head_request, 1) ;
        printf("%s\n", "---------------------");
        pthread_mutex_unlock(&lock);
        pthread_mutex_unlock(&navigate_sig) ; // Start Driving Thread
        //pthread_mutex_lock(&navigate_sig) ;
    }
}

void * driving_thread(){
    // arg needs to be modift to (link_list*), path calculate need redesign
    link_list uber, request ;
    uber = paired_data[0] ;
    request = paired_data[1] ;
    //int origin_x , origin_y, dest_x, dest_y ;
    printf("%d,%d -> %d,%d ==> ",
        uber.data[0], uber.data[1], request.data[0], request.data[1]) ;
    // printf("%d,%d -> %d,%d -> %d,%d\n",
    //     uber->src_x, uber->src_y, uber->src_x, request->src_y, request->src_x, request->src_y) ;

    #if PXA
    lcd_write_info_t display ;
    display.Count = sprintf((char*) display.Msg, "%d,%d -> %d,%d ==> %d,%d -> %d,%d -> %d,%d\n",
        val_p[0], val_p[1], val_p[2], val_p[3], val_p[0], val_p[1],val_p[0], val_p[3], val_p[2], val_p[3]) ;
    ioctl(lcd_fd, LCD_IOCTL_WRITE, &display) ;
    #endif

    // add enqueue() to return the used uber.

    pthread_exit( NULL );
}

link_list * dequeue(link_list * head, int idx){
	if(head != NULL){
		//printf("%d,%d\n", head->x, head->y) ;
        paired_data[idx] = *head ;
        //paired_data[idx+1] = head->y ;
		return head->next ;
	}
	else{
		printf("Queue is empty.\n") ;
		return head ;
	}
}

void enqueue_safe(link_list * head, int * vals, int priority){
    pthread_mutex_lock(&lock);
    head_request = enqueue(head, vals, priority) ;
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
