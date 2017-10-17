#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "seg.h"

#define MAX_RULE_NUMBER 100
#define MAX_SIZE 4
#define PXA 0

#if PXA
#include "asm-arm/arch-pxa/lib/creator_pxa270_lcd.h"
#endif

// Link list structure
typedef struct link_list{
	int val ;
	int priority ;
	struct  link_list * next ;
} link_list ;

// Function Declare
link_list * enqueue() ;
link_list * dequeue() ;
void show_queue() ;
int get_priority() ;
void blink() ;
void show_7seg() ;
void show_lcd() ;

// Global var declare
int rule_arr[MAX_RULE_NUMBER] = {} ;
int q_size = 0 ;

int main(int argc, char ** argv){
    const char * filename = argv[1] ;
	const char * rule_file = argv[2] ;
    FILE * fp = fopen(filename, "r") ;
    FILE * rule_fd = fopen(rule_file, "r") ;
	FILE * out = fopen("output.txt", "w") ;

	#if PXA
	int led_fd ;
	if((led_fd = open("/dev/lcd", O_RDWR)) < 0){
        printf("open failed\n") ;
        exit(-1) ;
    }
	unsigned short key = 0 ;
	ioctl(led_fd, KEY_IOCTL_CLEAR, key) ;
	printf("Init complete, press a buttom to start the program.\n") ;
	ioctl(led_fd, KEY_IOCTL_WAIT_CHAR, &key) ;
	#endif
	
	link_list * head = NULL ;
    int val ;
    clock_t begin_en = clock();

	// Read Rule File
	int rule_idx = 0 ;
	while(fscanf(rule_fd, "%d\n", &val) > 0){
		rule_arr[rule_idx] = val;
		rule_idx++ ;
	}


	//enqueue all elements.
	while(fscanf(fp, "%d\n", &val) > 0){
		int priority = get_priority(val, rule_idx) ;
    	//printf("val=%d, priority=%d\n", val, priority) ;
		head = enqueue(head, val, priority) ;	
		show_queue(head) ;
	}
	clock_t end_en = clock();
	printf("==========pop all=========\n") ;
	
	// Dequeue all elements
	clock_t begin_de = clock();
	while(head != NULL){
		link_list * tmp = head ;
		head = dequeue(head) ;
		free(tmp) ;
	}
	clock_t end_de = clock();
	
	fprintf(out, "Time spent (Enqueue): %.3lf\n", (double)(end_en - begin_en) / CLOCKS_PER_SEC) ;
	fprintf(out, "Time spent (Dequeue): %.3lf\n", (double)(end_de - begin_de) / CLOCKS_PER_SEC) ;
	fclose(fp) ;
	fclose(out) ;
	fclose(rule_fd) ;
	return 0 ;
}


link_list * enqueue(link_list * head, int val, int priority){
	// First check the q size
	if(q_size >= MAX_SIZE){
		//Q is full: pop & free
		link_list * ptr = head ;
		head = dequeue(head);
		free(ptr) ;
	}
	q_size ++ ;
	link_list * node = (link_list*) malloc(sizeof(link_list)) ;
	node->val = val ;
	node->priority = priority ;
	node->next = NULL ;
	//printf("Start enqueue...\n") ;
	if(head == NULL){
		//printf("Create queue.\n") ;
		return node ;
	}
	else{
		if(priority < head->priority){
			//printf("Insert to head\n") ;
			node->next = head ;
			return node ;
		}
		link_list * ptr = head ;
		while(ptr->next != NULL){
			// Check priority
			if(priority < (ptr->next)->priority){
				//printf("Insert to middle\n") ;
				node->next = ptr->next ;
				ptr->next = node ;
				return head ;
			}
			ptr = ptr->next ;
		}
		// Next node is NULL
		//printf("Insert to tail.\n") ;
		ptr->next = node ;
	}
	return head ;
}

link_list * dequeue(link_list * head){
	if(head != NULL){
		q_size-- ;
		printf("Pop out: val=%d, priority=%d\n", head->val, head->priority) ;
		printf("hex:%lx\n", seg_data_gen(head->priority)) ;
		return head->next ;
	}
	else{
		printf("Queue is empty.\n") ;
		return head ;
	}
}

// Print the queue
void show_queue(link_list * head){
	link_list * ptr = head ;
	while(ptr != NULL){
		printf("%d->", ptr->val) ;
		ptr = ptr->next ;
	}
	printf("NULL\n") ;
}

// Get the priority of this number
int get_priority(int val, int rule_cnt){
	int i = 0 ;
	for(i = 0; i<rule_cnt; i++){
		if(val%rule_arr[i] == 0){
			return i ;
		}
	}
	return MAX_RULE_NUMBER ;
}

#if PXA
void blink(int led_fd){
	unsigned short data ;
	data=LED_ALL_ON ;
    ioctl (led_fd , LED_IOCTL_SET, &data );
    printf("Turn on all LED lamps\n");
    sleep (0.1);

    data=LED_ALL_OFF ;
    ioctl (led_fd , LED_IOCTL_SET, &data );
    printf("Turn off all LED lamps\n");
	sleep(0.1) ;

	ioctl (led_fd , LED_IOCTL_SET, &data );
    printf("Turn on all LED lamps\n");
    sleep (0.1);

    data=LED_ALL_OFF ;
    ioctl (led_fd , LED_IOCTL_SET, &data );
    printf("Turn off all LED lamps\n");
}

void show_7seg(int lcd_fd, int rule_num){
	_7seg_info_t data ;
	ioctl(lcd_fd, _7SEG_IOCTL_ON, NULL) ;
	data.Mode = _7SEG_MODE_PATTERN ;
	data.Which = 15 ;
	data.Value = seg_data_gen(rule_num) ;
	ioctl(lcd_fd, _7SEG_IOCTL_SET, &data) ;
	sleep(1) ;
	ioctl(lcd_fd, _7SEG_IOCTL_OFF, NULL) ;
}

void show_lcd(int lcd_fd, int val){
	lcd_write_info_t display ;
	ioctl(lcd_fd, LCD_IOCTL_CLEAR, NULL) ;
	display.Count = sprintf((char*) display.Msg, "%d", val) ;
	ioctl(lcd_fd, LCD_IOCTL_WRITE, &display) ;
}
#endif

