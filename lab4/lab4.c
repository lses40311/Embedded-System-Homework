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
	int val ;
    int x ;
    int y ;
	int priority ;
	struct  link_list * next ;
} link_list ;
//Link list structure

// Function Declare
void * uber_thread() ;
void * customer_thread() ;
void * driving_thread() ;
void enqueue_safe() ;
link_list * enqueue() ;
link_list * dequeue() ;

// Global Variables
link_list * head_request = NULL ;
link_list * head_uber = NULL ;
sem_t * semaphore ;
pthread_mutex_t lock, navigate_sig;
int * paired_data ;

// main
int main(int argc, char ** argv){
    if(argc != 3){
        printf("Wrong Parameters\nUsage: ./a uberfile customerrequest\n");
        exit(EXIT_FAILURE) ;
    }
    const char * uber_loc_file = argv[1] ;
    const char * user_request_file = argv[2] ;
    paired_data = malloc(4 * sizeof(int));

    // Initialize Semaphore & mutex
    //sem_init(&semaphore, 0, 0);
    semaphore = sem_open("test",O_CREAT, 0644, 0) ;
    pthread_mutex_init(&lock, NULL) ;
    pthread_mutex_init(&navigate_sig, NULL) ;
    pthread_mutex_lock(&navigate_sig) ;

    //create two worker threads
    pthread_t* thread_handles = malloc(3 * sizeof(pthread_t));
    pthread_create(&thread_handles[0], NULL, customer_thread, (void*) user_request_file);
    pthread_create(&thread_handles[1], NULL, uber_thread, (void*) uber_loc_file);
   
    // Create a navigation thread
    while(1){ 
        pthread_mutex_lock(&navigate_sig) ;
        printf("Create navigation thread\n") ;
        pthread_create(&thread_handles[3], NULL, driving_thread, (void*) paired_data);
        pthread_detach(thread_handles[3]) ;
        pthread_mutex_unlock(&navigate_sig) ;
    }

    // Prevent the main thread from terminated.
    sleep(300) ;
    
    // Close & unlink sem_t
    sem_unlink("test") ;
    sem_close(semaphore) ;

    return 0 ;
}

void * customer_thread(void * arg){
    char *val_p = (char *) arg ;
    int x, y , priority ;
    FILE * fp = fopen(val_p, "r") ;
    while(fscanf(fp, "%d %d %d\n", &x, &y, &priority) > 0){
        printf("val=%d,%d, priority=%d\n", x, y, priority) ;
        enqueue_safe(head_request, x, y, priority);
		// show_queue(head) ;
    }
    
    // enter request from STDIN/Button
    while(1){
        #if PXA
        // From Button

        #else
        // From STDIN
        printf("Enter request:\n") ;
        scanf("%d %d %d", &x, &y, &priority) ;
        printf("Read val=%d,%d with priority=%d\n", x, y, priority) ;
        #endif
        
        enqueue_safe(head_request, x, y, priority);
    }
}

void * uber_thread(void * arg){
    char *val_p = (char *) arg ;
    int x, y , priority, uber_cnt ;
    uber_cnt = 0 ;
    FILE * fp = fopen(val_p, "r") ;
    while(fscanf(fp, "%d %d\n", &x, &y) > 0){
        printf("val=%d,%d\n", x, y) ;
        head_uber = enqueue(head_uber, x, y);
		uber_cnt++ ;
        //show_queue(head) ;
	}
    int sem_val ;
    while (1) {
        sem_wait(semaphore) ;
        //sem_val = sem_getvalue(&semaphore, &sem_val) ;
        pthread_mutex_lock(&lock);
        printf("%s\n", "---------------------");
        //printf("%s, %d\n", "Find Matchd", sem_val);
        
        #if newUBER
        if(uber_cnt <= 0){
            // Need more UBER
            printf("Give me more uber:\n") ;
            scanf("%d %d", &x, &y) ;
            head_uber = enqueue(head_uber, x, y);
            uber_cnt++ ;
        }
        #endif
        head_uber = dequeue(head_uber, 0) ;
        head_request = dequeue(head_request, 2) ;
        uber_cnt-- ;
        printf("%s\n", "---------------------");
        pthread_mutex_unlock(&lock);
        pthread_mutex_unlock(&navigate_sig) ; // Start Driving Thread
        pthread_mutex_lock(&navigate_sig) ;
    }
}

void * driving_thread(void * arg){
    int *val_p = (int *) arg ;
    int origin_x , origin_y, dest_x, dest_y ;
    printf("%d,%d -> %d,%d ==> ", val_p[0], val_p[1], val_p[2], val_p[3]) ;
    printf("%d,%d -> %d,%d -> %d,%d\n", val_p[0], val_p[1],val_p[0], val_p[3],  val_p[2], val_p[3]) ;
    
    #if PXA
    lcd_write_info_t display ;
    display.Count = sprintf((char*) display.Msg, "%d,%d -> %d,%d ==> %d,%d -> %d,%d -> %d,%d\n", 
        val_p[0], val_p[1], val_p[2], val_p[3], val_p[0], val_p[1],val_p[0], val_p[3], val_p[2], val_p[3]) ;
    ioctl(lcd_fd, LCD_IOCTL_WRITE, &display) ;
    #endif
    
    pthread_exit( NULL );
}

void enqueue_safe(link_list * head, int x, int y, int priority){
    pthread_mutex_lock(&lock);
    head_request = enqueue(head, x, y, priority) ;
    pthread_mutex_unlock(&lock);
    sem_post(semaphore);
}

// Enqueue
link_list * enqueue(link_list * head, int x, int y, int priority){
	link_list * node = (link_list*) malloc(sizeof(link_list)) ;
	node->x = x ;
    node->y = y ;
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

// Dequeue
link_list * dequeue(link_list * head, int idx){
	if(head != NULL){
		//printf("%d,%d\n", head->x, head->y) ;
        paired_data[idx] = head->x ;
        paired_data[idx+1] = head->y ;
		return head->next ;
	}
	else{
		printf("Queue is empty.\n") ;
		return head ;
	}
}
