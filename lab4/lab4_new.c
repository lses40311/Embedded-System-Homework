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

int main(){
#if PXA
    int lcd_fd ;
    if((lcd_fd = open("/dev/lcd", O_RDWR)) < 0){
        printf("open failed\n") ;
        exit(-1) ;
    }
    unsigned short key = 0 ;
    ioctl(lcd_fd, KEY_IOCTL_CLEAR, key) ;
    printf("Init complete, press a buttom to start the program.\n") ;
    int i = ;
    for(i =0; i<4; i++){
        ioctl(lcd_fd, KEY_IOCTL_WAIT_CHAR, &key) ;
        printf("%hu\n", key) ;
    }
    //ioctl(lcd_fd, LCD_IOCTL_CLEAR, NULL) ;
    printf("Program Start.\n") ;
    #endif
}





