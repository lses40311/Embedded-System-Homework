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
#include <sys/time.h>


#define PXA 0
#define newUBER 0
#define MAX_AXIS_X 9
#define MAX_AXIS_Y 9
#define MY_KEY 5588

#if PXA
#include "seg_show_number.h"
#include "asm-arm/arch-pxa/lib/creator_pxa270_lcd.h"
#endif

// Link list structure
typedef struct link_list{
    int data[4] ;
    int priority ;
    int id ;
    struct  link_list * next ;
} link_list ;
//Link list structure

// Function Declare
void * uber_thread() ;
void * customer_thread() ;
void * driving_thread() ;
void * center_thread() ;
void enqueue_safe() ;
void show_queue() ;
char * get_path_str() ;
link_list * enqueue() ;
link_list * dequeue() ;
int my_sem_init() ;
int my_sem_remove() ;
int my_sem_post() ;
int my_sem_wait() ;
void blink() ;
void show_7seg() ;
void timer_handler() ;


// Global Variables
link_list * head_request = NULL ;
link_list * head_uber = NULL ;
pthread_mutex_t lock, navigate_start, data_passing_done;
int * input_data ;
link_list * paired_data ;
int lcd_fd ;
int pfd[2] ;
int request_sem, uber_sem, intersection_sem ; // originally sem_t
int driving_thread_num ;
int time_cnt ;

// Timer signal
void timer_handler(int signum) {
    time_cnt ++ ;
}

// Handles Ctrl + c
void intHandler(int dummy) {
    my_sem_remove(request_sem, 1) ;
    my_sem_remove(uber_sem, 1) ;
    my_sem_remove(intersection_sem, MAX_AXIS_X*MAX_AXIS_Y) ;
    exit(0) ;
}

int trans_position(int x, int y){
    return (x-1) * 9 + (y-1) ;
}

int main(int argc, char ** argv){
    time_cnt = 0 ;
    if(argc != 2){
        printf("Wrong Parameters\nUsage: ./a uberfile\n");
        exit(EXIT_FAILURE) ;
    }
    const char * uber_loc_file = argv[1] ;
    paired_data = malloc(2 * sizeof(link_list));

    // remove semaphore if system interupted
    //signal(SIGINT, intHandler);

    // Initialize Semaphore & mutex
    if((request_sem = my_sem_init(MY_KEY, 1, 0)) < 0){
        printf("%s\n", "open request_sem error") ;
        intHandler(0) ;
        exit(1) ;
    }
    else{
        printf("%s\n", "Open request_sem.") ;
    }
    if((intersection_sem = my_sem_init(MY_KEY+2, MAX_AXIS_X*MAX_AXIS_Y, 1)) < 0){
        printf("%s\n", "open intersection_sem error") ;
        intHandler(0) ;
        exit(1) ;
    }
    else{
        printf("%s\n", "Open intersection_sem.") ;
    }

    pthread_mutex_init(&lock, NULL) ;
    pthread_mutex_init(&navigate_start, NULL) ;
    pthread_mutex_init(&data_passing_done, NULL) ;
    pthread_mutex_lock(&navigate_start) ;
    pthread_mutex_lock(&data_passing_done) ;

    // Setting signal
    struct sigaction sa ;
    struct itimerval timer ;
    memset(&sa, 0, sizeof(sa)) ;
    sa.sa_handler = &timer_handler ;
    sigaction () ;

    // Creating pipe
    if (pipe(pfd) == -1){
        printf("Creating pipe failed\n") ;
    }

    #if PXA
    // Initialize PXA I/O
	unsigned short key ;
    if((lcd_fd = open("/dev/lcd", O_RDWR)) < 0){
        printf("open failed\n") ;
        exit(-1) ;
    }
    ioctl(lcd_fd, _7SEG_IOCTL_ON, NULL) ;
	printf("LCD Initialization Success.\n") ;
    #endif

    //create two worker threads
    pthread_t* thread_handles = malloc(4 * sizeof(pthread_t));
    pthread_create(&thread_handles[0], NULL, customer_thread, NULL);
    pthread_create(&thread_handles[1], NULL, uber_thread, (void*) uber_loc_file );
    pthread_create(&thread_handles[3], NULL, center_thread, NULL);

    // Create a navigation thread
    driving_thread_num = 0 ;
    while(1){
        pthread_mutex_lock(&navigate_start) ;
        printf("Create driving thread\n") ;
        link_list * args = malloc(2 * sizeof(link_list));
        memcpy(args, paired_data, 2*sizeof(link_list)) ;
        if (pthread_create(&thread_handles[2], NULL, driving_thread, args) >= 0){
            driving_thread_num ++ ;
			printf("Successfully create driving thread#%d\n", driving_thread_num) ;
			#if PXA
            show_7seg(lcd_fd, driving_thread_num) ;
			#endif
			//free(args) ;
        }
        pthread_detach(thread_handles[2]) ;
        pthread_mutex_unlock(&data_passing_done) ;
    }

    return 0 ;
}

void * customer_thread(){
    int priority = 0 ;
    while (1) {
        printf("Please Enter 4 numbers as the customer's request..\n") ;
        int ** requests ;
        requests = malloc(10*sizeof(int*)) ;
        int cnt = 0 ;
        int i ;
        while(1){
            requests[cnt] = malloc(4*sizeof(int)) ;
            scanf("%d %d %d %d", &requests[cnt][0], &requests[cnt][1], &requests[cnt][2], &requests[cnt][3]) ;
            if(requests[cnt][0] == -1){
                break ;
            }
            cnt++ ;
        }
        for(i=0; i<cnt; i++){
            enqueue_safe(&head_request, requests[i], priority, 0);
            free(requests[i]) ;
            my_sem_post(request_sem, 0);
		}
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
        head_uber = enqueue(head_uber, coordinates, 0, uber_cnt);
        my_sem_wait(intersection_sem, trans_position(coordinates[0], coordinates[1]), 0) ;
		uber_cnt++ ;
	}

    // Initialize Semaphore with uber number
    //sem_init(&uber_sem, 0, uber_cnt) ;
    if((uber_sem = my_sem_init( MY_KEY+1, 1, uber_cnt)) < 0){
        printf("%s\n", "open uber_sem error") ;
        exit(1) ;
    }

    while (1) {
        my_sem_wait(request_sem, 0, 0);
        my_sem_wait(uber_sem, 0, 0);
        pthread_mutex_lock(&lock);
        printf("%s\n", "Find a car.");
        head_uber = dequeue(head_uber, paired_data, 0) ;
        head_request = dequeue(head_request, paired_data, 1) ;
        pthread_mutex_unlock(&lock);
        pthread_mutex_unlock(&navigate_start) ; // Start Driving Thread
        pthread_mutex_lock(&data_passing_done) ;
    }
}

void * driving_thread(void * args){
    printf("%s\n", "driving thread");
    int sleep_time = 2 ;
    int i ;
    link_list uber, request ;
    link_list * tmp = (link_list *) args ;
    uber = tmp[0] ;
    request = tmp[1] ;
    int car_id = tmp[0].id ;
    int * x = malloc(5*sizeof(int)) ;
    int * y = malloc(5*sizeof(int)) ;
    // Planning Path
    x[0] = uber.data[0] ;
    y[0] = uber.data[1] ;
    x[1] = uber.data[0] ;
    y[1] = request.data[1] ;
    x[2] = request.data[0] ;
    y[2] = request.data[1] ;
    x[3] = request.data[0] ;
    y[3] = request.data[3] ;
    x[4] = request.data[2] ;
    y[4] = request.data[3] ;
    char * target_str = malloc(100*sizeof(char));
    char * str = malloc(10*sizeof(char));
    int distance = abs(uber.data[0] - request.data[0]) +
        abs( uber.data[1] - request.data[1]) +
        abs(request.data[0] - request.data[2]) +
        abs(request.data[1] - request.data[3]) ;
    sprintf(target_str, "car %d dist=%d: %d,%d->%d,%d->%d,%d==>%d,%d",
        car_id, distance, uber.data[0], uber.data[1], request.data[0], request.data[1], request.data[2], request.data[3], uber.data[0], uber.data[1]);
    for (i = 1; i < 5; i++) {
        sleep(sleep_time) ;
        int next_intersection_id = trans_position(x[i], y[i]) ;
        int current_intersection_id = trans_position(x[i-1], y[i-1]) ;
        if (next_intersection_id == current_intersection_id){
            sprintf(str, "->%d,%d ", x[i], y[i]) ;
            target_str = strcat( target_str, str ) ;
            printf("%s\n", target_str) ;
            continue ;
        }
        while((my_sem_wait(intersection_sem, next_intersection_id, IPC_NOWAIT) < 0)) {
            target_str = strcat( target_str, "->wait" ) ;
            printf("%s\n", target_str) ;
            #if PXA
            blink(lcd_fd) ; // 4 seconds
            #else
            sleep(sleep_time) ;
            #endif
        }
        my_sem_post(intersection_sem, current_intersection_id) ;
        sprintf(str, "->%d,%d ", x[i], y[i]) ;
        target_str = strcat( target_str, str ) ;
        printf("%s\n", target_str) ;
    }
	printf("--------------------------Car %d done-------------------------------\n", car_id) ;
    sprintf(target_str, "%s\n", target_str) ;
    printf("%s", target_str) ;
    write(pfd[1], target_str, strlen(target_str)) ;
	printf("-------------------------------------------------------------------\n") ;

    int * uber_return = malloc(4*sizeof(int)) ;
    uber_return[0] = request.data[2] ;
    uber_return[1] = request.data[3] ;
    uber_return[2] = 0 ;
    uber_return[3] = 0 ;
    enqueue_safe(&head_uber, uber_return, 0, car_id) ;
    //sem_post(&uber_sem) ;
    my_sem_post(uber_sem, 0) ;

    free(target_str) ;
    free(str) ;
    free(uber_return) ;
    driving_thread_num-- ;
	#if PXA
	show_7seg(lcd_fd, driving_thread_num) ;
	#endif
	pthread_exit(NULL);
}

void * center_thread(){
    int ret ;
    char buf[1024] ;
    while(1){
		memset(buf, 0, 1024) ;
        sleep(45) ;
        ret = read(pfd[0], buf, 1024) ;
        printf("--------------------------Center Message-------------------------------\n") ;
        printf("Read %d chars from pipe.\n", ret) ;
		printf("%s", buf) ;
        printf("-----------------------------------------------------------------------\n") ;
    }
}

link_list * dequeue(link_list * head, link_list * result, int idx){
	if(head != NULL){
        show_queue(head) ;
        result[idx] = *head ;
		return head->next ;
	}
	else{
		printf("Queue is empty.\n") ;
		return head ;
	}
}

void enqueue_safe(link_list ** head, int * vals, int priority, int id){
    pthread_mutex_lock(&lock);
    *head = enqueue(*head, vals, priority, id) ;
    pthread_mutex_unlock(&lock);
}

link_list * enqueue(link_list * head, int * vals, int priority, int id){
	link_list * node = (link_list*) malloc(sizeof(link_list)) ;
    int i ;
    for (i = 0; i < 4; i++) {
        node->data[i] = vals[i] ;
    }
    node->id = id ;
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

int my_sem_wait(int s, int sem_idx, int flag){
    struct sembuf sop ;
    sop.sem_num = sem_idx ;
    sop.sem_op = -1 ;
    sop.sem_flg = flag ;
    //printf("waiting for%d\n", sem_idx);
    if(semop(s, &sop, 1) < 0){
        printf("Waiting for sem %d failed. Sem idx=%d\n", s, sem_idx);
        return -1 ;
    }
    else{
        return 0;
    }
}

#if PXA
void blink(int led_fd){
	unsigned short data ;
	data=LED_ALL_ON ;
    ioctl (led_fd , LED_IOCTL_SET, &data );
    //printf("Turn on all LED lamps\n");
    sleep (1);

    data=LED_ALL_OFF ;
    ioctl (led_fd , LED_IOCTL_SET, &data );
    //printf("Turn off all LED lamps\n");
	sleep(1) ;

	data = LED_ALL_ON ;
	ioctl (led_fd , LED_IOCTL_SET, &data );
    sleep (1);

    data=LED_ALL_OFF ;
    ioctl (led_fd , LED_IOCTL_SET, &data );
    sleep(1) ;
}

void show_7seg(int lcd_fd, int number){
	_7seg_info_t data ;
	//ioctl(lcd_fd, _7SEG_IOCTL_ON, NULL) ;
	data.Mode = _7SEG_MODE_PATTERN ;
	data.Value = seg_data_gen(number) ;
	data.Which = _7SEG_D5_INDEX | _7SEG_D6_INDEX | _7SEG_D7_INDEX | _7SEG_D8_INDEX ;
	ioctl(lcd_fd, _7SEG_IOCTL_SET, &data) ;
	//sleep(1) ;
	//ioctl(lcd_fd, _7SEG_IOCTL_OFF, NULL) ;
}
#endif
