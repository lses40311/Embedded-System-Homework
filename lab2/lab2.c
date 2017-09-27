#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    while(fscanf(fp, "%lf,%d\n", &val, &priority) > 0){
        printf("%lf, %d\n", val, priority) ;
    	head = enqueue(head, val, priority) ;	
	}
	
	printf("===================\n") ;

	while(head != NULL){
		link_list * tmp = head ;
		head = dequeue(head) ;
		free(tmp) ;
	}

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
