#include "MC.hpp"

void *threadMobileCounter(void *threadid)
{
	while (1){
			pthread_mutex_lock(&MC_recv_List_lock);
			for (int i = 0; i < MC_recv_List.size(); i++){
				cout << MC_recv_List.front() << endl;
				MC_recv_List.pop();
			}
			pthread_mutex_unlock(&MC_recv_List_lock);
		
	}

	pthread_exit(NULL);
	
}