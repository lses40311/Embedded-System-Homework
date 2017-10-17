#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_RULE_NUMBER 100
#define MAX_SIZE 4
#define PXA 1

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


// Global var declare
int rule_arr[MAX_RULE_NUMBER] = {} ;
int q_size = 0 ;

int main(int argc, char ** argv){
    const char * filename = argv[1] ;
	const char * rule_file = argv[2] ;
    FILE * fp = fopen(filename, "r") ;
    FILE * rule_fd = fopen(rule_file, "r") ;
	FILE * out = fopen("output.txt", "w") ;
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
