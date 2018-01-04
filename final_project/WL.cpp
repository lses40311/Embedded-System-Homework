#include "WL.hpp"
#include <unistd.h>


int statistic_list[4];

void *threadWaitingList(void *threadid)
{
	struct sigaction report_sa;
	struct itimerval report_timer;
	memset(&report_sa, 0, sizeof(report_sa));
	report_sa.sa_handler = &report_handler;
	sigaction(SIGVTALRM, &report_sa, NULL);
	report_timer.it_value.tv_sec = 5;
	report_timer.it_value.tv_usec = 0;
	report_timer.it_interval.tv_sec = 5;
	report_timer.it_interval.tv_usec = 0;
	setitimer(ITIMER_VIRTUAL, &report_timer, NULL);
	
	
	int order_list[4];
	int lock_order_list[4];
	
	// Initial list
	for (int i = 0; i < 4; i++){
		order_list[i] = 0;
		lock_order_list[i] = 0;
		statistic_list[i] = 0;
	}
	
	string show_str = "";
	
	int lock_cook_idx = -1;
	
	while (1){
		/***  Update Order List  ***/
		queue<char> in_order;
		
		pthread_mutex_lock(&Order_List_lock);
		int ord_loop_num = Order_recv_List.size();
		for (int i = 0; i < ord_loop_num; i++){
			in_order.push(Order_recv_List.front());
			Order_recv_List.pop();
		}
		pthread_mutex_unlock(&Order_List_lock);
		
		if (lock_cook_idx == -1){
			int in_ord_loop_num = in_order.size();
			for (int i = 0; i < in_ord_loop_num; i++){
				int order_idx = in_order.front() - '0';
				order_list[order_idx]++;
				
				statistic_list[order_idx]++;
				in_order.pop();
			}
		}
		else{
			int in_ord_loop_num = in_order.size();
			for (int i = 0; i < in_ord_loop_num; i++){

				int order_idx = in_order.front() - '0';
				if (order_idx == lock_cook_idx)
					lock_order_list[order_idx]++;
				else
					order_list[order_idx]++;
				
				statistic_list[order_idx]++;
				in_order.pop();
			}
		}
		
		
		/***  Input key and clean order  ***/
		queue<char> in_commend;
		
		pthread_mutex_lock(&WL_recv_List_lock);
		int wl_loop_num = WL_recv_List.size();
		for (int i = 0; i < wl_loop_num; i++){
			in_commend.push(WL_recv_List.front());
			WL_recv_List.pop();
		}
		pthread_mutex_unlock(&WL_recv_List_lock);
		
		int com_loop_num = in_commend.size();
		for (int i = 0; i < com_loop_num; i++){
			if (in_commend.front() == '*'){				// Clear
				show_str = "";
				lock_cook_idx == -1;
				
				in_order = queue<char>();
				pthread_mutex_lock(&Order_List_lock);
				Order_recv_List = queue<char>();
				pthread_mutex_unlock(&Order_List_lock);
				
				for (int i = 0; i < 4; i++){
					order_list[i] = 0;
					lock_order_list[i] = 0;
				}
			}
			else{
				if (lock_cook_idx == -1 && in_commend.front() <= '3' && in_commend.front() >= '0'){
					// Lock and start cooking
					lock_cook_idx = in_commend.front() - '0';
				}
				else if (in_commend.front() <= '3' && in_commend.front() >= '0'){
					// Unlock and update
					order_list[lock_cook_idx] = lock_order_list[lock_cook_idx];
					lock_order_list[lock_cook_idx] = 0;
					
					lock_cook_idx = -1;
				}
			}
			
			in_commend.pop();
		}
		
		
		
		/***  Organize output  ***/
		show_str = "";
		if (lock_cook_idx != -1){
			show_str += "== Cooking ==\n";
			if (lock_cook_idx == 0){
				show_str += "Water # ";
				show_str += to_string(order_list[0]);
				show_str += "\n";
			}
			else if (lock_cook_idx == 1){
				show_str += "Subway # ";
				show_str += to_string(order_list[1]);
				show_str += "\n";
			}
			else if (lock_cook_idx == 2){
				show_str += "Coffee # ";
				show_str += to_string(order_list[2]);
				show_str += "\n";
			}
			else if (lock_cook_idx == 3){
				show_str += "Sandwich # ";
				show_str += to_string(order_list[3]);
				show_str += "\n";
			}
			show_str += "=============\n";
		}
		
		show_str += "== Waiting ==\n";
		
		show_str += "0. Water # ";
		if (lock_cook_idx == 0)
			show_str += to_string(lock_order_list[0]);
		else
		show_str += to_string(order_list[0]);
		show_str += "\n";
		
		show_str += "1. Subway # ";
		if (lock_cook_idx == 1)
			show_str += to_string(lock_order_list[1]);
		else
			show_str += to_string(order_list[1]);
		show_str += "\n";
		
		show_str += "2. Coffee # ";
		if (lock_cook_idx == 2)
			show_str += to_string(lock_order_list[2]);
		else
			show_str += to_string(order_list[2]);
		show_str += "\n";
		
		show_str += "3. Sandwich # ";
		if (lock_cook_idx == 3)
			show_str += to_string(lock_order_list[3]);
		else
			show_str += to_string(order_list[3]);
		show_str += "\n";
		
		
		pthread_mutex_lock(&WL_send_List_lock);
		WL_send_List = show_str;
		pthread_mutex_unlock(&WL_send_List_lock);
		
		
		
		usleep(WL_WAIT_TIME);

	}

	pthread_exit(NULL);
	
}


void report_handler(int signum){
	int idx;
	printf("\n==== Report Start =====\n");
	cout << "  Water # " << statistic_list[0] << endl;
	cout << "  Subway # " << statistic_list[1] << endl;
	cout << "  Coffee # " << statistic_list[2] << endl;
	cout << "  Sandwich # " << statistic_list[3] << endl;
	printf("==== Report End =====\n");
}
