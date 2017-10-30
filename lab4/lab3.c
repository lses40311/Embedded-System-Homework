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
#define PXA 1

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
	int lcd_fd ;
	if((lcd_fd = open("/dev/lcd", O_RDWR)) < 0){
        printf("open failed\n") ;
        exit(-1) ;
    }
	unsigned short key = 0 ;
	ioctl(lcd_fd, KEY_IOCTL_CLEAR, key) ;
	printf("Init complete, press a buttom to start the program.\n") ;
	ioctl(lcd_fd, KEY_IOCTL_WAIT_CHAR, &key) ;
	ioctl(lcd_fd, _7SEG_IOCTL_ON, NULL) ;
	ioctl(lcd_fd, LCD_IOCTL_CLEAR, NULL) ;
	printf("Program Start.\n") ;
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
		head = enqueue(head, val, priority, lcd_fd) ;	
		// show_queue(head) ;
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


link_list * enqueue(link_list * head, int val, int priority, int lcd_fd){
	// First check the q size
	if(q_size >= MAX_SIZE){
		//Q is full: pop & free
		link_list * ptr = head ;
		head = dequeue(head, lcd_fd);
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

link_list * dequeue(link_list * head, int lcd_fd){
	if(head != NULL){
		q_size-- ;
		printf("%d, rule:%d\n", head->val, head->priority+1) ;
		//printf("Pop out: val=%d, priority=%d\n", head->val, head->priority) ;
		#if PXA
		//show_lcd(lcd_fd, head->val) ;
		show_queue(head, lcd_fd) ;
		show_7seg(lcd_fd, head->priority) ;
		if((head->priority) == 0){
			blink(lcd_fd) ;
		}
		#endif
		return head->next ;
	}
	else{
		printf("Queue is empty.\n") ;
		return head ;
	}
}

// Print the queue
void show_queue(link_list * head, int lcd_fd){
	link_list * ptr = head ;
	lcd_write_info_t display ;
	
	ioctl(lcd_fd, LCD_IOCTL_CLEAR, NULL) ;
	while(ptr != NULL){
		//printf("%d->", ptr->val) ;
		display.Count = sprintf((char*) display.Msg, "%d ", ptr->val) ;
		ioctl(lcd_fd, LCD_IOCTL_WRITE, &display) ;
		ptr = ptr->next ;
	}
	//printf("NULL\n") ;
}

// Get the priority of this number
int get_priority(int val, int rule_cnt){
	int i = 0 ;
	for(i = 0; i<rule_cnt; i++){
		if(val%rule_arr[i] == 0){
			return i ;
		}
	}
	return rule_cnt ;
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
}

void show_7seg(int lcd_fd, int rule_num){
	_7seg_info_t data ;
	//ioctl(lcd_fd, _7SEG_IOCTL_ON, NULL) ;
	data.Mode = _7SEG_MODE_PATTERN ;
	data.Value = seg_data_gen(rule_num) ;
	data.Which = _7SEG_D5_INDEX | _7SEG_D6_INDEX | _7SEG_D7_INDEX | _7SEG_D8_INDEX ;
	ioctl(lcd_fd, _7SEG_IOCTL_SET, &data) ;
	sleep(1) ;
	//ioctl(lcd_fd, _7SEG_IOCTL_OFF, NULL) ;
}

void show_lcd(int lcd_fd, int val){
	lcd_write_info_t display ;
	// ioctl(lcd_fd, LCD_IOCTL_CLEAR, NULL) ;
	display.Count = sprintf((char*) display.Msg, " %d", val) ;
	ioctl(lcd_fd, LCD_IOCTL_WRITE, &display) ;
}
#endif

