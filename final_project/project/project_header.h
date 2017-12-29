/* 
 * project_header.h
 */
 

void *threadMobileCounter(void *threadid);
void *threadWaitingList(void *threadid);
void *threadPOS(void *threadid);
void addOrder(int order_index, char order_name[]);
