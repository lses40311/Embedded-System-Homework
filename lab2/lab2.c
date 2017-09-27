#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct link_list{
	double val ;
	int priority ;
	struct  link_list * next ;
} link_list ;

link_list * enqueue() ;
link_list * dequeue() ;

int main(int argc, char ** argv){
    const char * filename = argv[1] ;
    FILE * fp = fopen(filename, "r") ;
	link_list * head ;
    int priority ;
    double val ;
    clock_t begin_en = clock();
	//enqueue all elements.
	while(fscanf(fp, "%lf,%d\n", &val, &priority) > 0){
        printf("%lf, %d\n", val, priority) ;
    	head = enqueue(head, val, priority) ;	
	}
	clock_t end_en = clock();
	printf("===================\n") ;
	
	// Dequeue all elements
	clock_t begin_de = clock();
	while(head != NULL){
		link_list * tmp = head ;
		head = dequeue(head) ;
		free(tmp) ;
	}
	clock_t end_de = clock();
	
	printf("Time spent (Enqueue): %.3lf\n", (double)(end_en - begin_en) / CLOCKS_PER_SEC) ;
	printf("Time spent (Dequeue): %.3lf\n", (double)(end_de - begin_de) / CLOCKS_PER_SEC) ;
	fclose(fp) ;
	return 0 ;
}


link_list * enqueue(link_list * head, double val, int priority){
	link_list * node = (link_list*) malloc(sizeof(link_list)) ;
	node->val = val ;
	node->priority = priority ;
	node->next = NULL ;
	printf("Start enqueue...\n") ;
	if(head == NULL){
		printf("Create queue.\n") ;
		return node ;
	}
	else{
		if(priority <= head->priority){
			printf("Insert to head\n") ;
			node->next = head ;
			return node ;
		}
		link_list * ptr = head ;
		while(ptr->next != NULL){
			// Check priority
			if(priority <= (ptr->next)->priority){
				printf("Insert to middle\n") ;
				node->next = ptr->next ;
				ptr->next = node ;
				return head ;
			}
			ptr = ptr->next ;
		}
		// Next node is NULL
		printf("Insert to tail.\n") ;
		ptr->next = node ;
	}
	return head ;
}

link_list * dequeue(link_list * head){
	if(head != NULL){
		printf("Pop out: val=%lf, priority=%d\n", head->val, head->priority) ;
		return head->next ;
	}
	else{
		printf("Queue is empty.\n") ;
		return head ;
	}
}
