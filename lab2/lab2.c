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

int main(){
	double arr[256] ;
	arr[0] = 10.5454 ;
	arr[1] = 0.55878 ;
	arr[2] = 50.54 ;
	int p[3] = {3 , 1 , 2} ;
	int len = 3 ;
	int i ;
	link_list * head ;
	for(i=0; i<len ; i++){
		head = enqueue(head, arr[i], p[i]) ;
	}
	for(i=0; i< len ; i++){
		link_list * tmp = head ;
		head = dequeue(head) ;
		free(tmp) ;
	}
	return 0 ;
}


link_list * enqueue(link_list * head, double val, int priority){
	link_list * node = (link_list*) malloc(sizeof(link_list)) ;
	node->val = val ;
	node->priority = priority ;
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
		}
		// Next node is NULL
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
