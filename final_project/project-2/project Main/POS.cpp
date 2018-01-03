#include "POS.hpp"


void *threadPOS(void *threadid)
{
	string show_str;
	queue<char> queue_list;
	queue<char> queue_num;
	bool is_num = false;
	bool pre_is_num = false;
	char pre_order = 0;
	bool is_start = true;
	
	while (1){
		queue<char> in_commend;
		
		pthread_mutex_lock(&POS_recv_List_lock);
		for (int i = 0; i < POS_recv_List.size(); i++){
			in_commend.push(POS_recv_List.front());
			POS_recv_List.pop();
		}
		pthread_mutex_unlock(&POS_recv_List_lock);
		
		for (int i = 0; i < in_commend.size(); i++){
			
			if (in_commend.front() == '*'){	// Clear
				show_str = "";
				queue_list = queue<char>();
				pre_is_num = is_num;
				is_num = false;
				is_start = true;
				pre_order = 0;
			}
			else{
				if (pre_order == 0){
					pre_is_num = is_num;
					is_num = false;
				}
				
				if (is_num){
					if (in_commend.front() <= '9' && in_commend.front() >='0'){
						queue_num.push(in_commend.front());
						show_str += in_commend.front();
					}
					else if (in_commend.front() == '#'){
						// Handling the num
						if (queue_num.size() == 0){
							show_str += "1";
						}
						else{
							int num = 0;
							for (int j = 0; j < queue_num.size(); j++){
								int digit = queue_num.size()-1;
								int ten_pow = 1;
								for (int k = 0; k < digit; k++){
									ten_pow *= 10;
								}
								num += ten_pow*(queue_num.front()-'0');
								queue_num.pop();
							}
							
							if (num == 0){
								queue_list.pop();
							}
							else{
								for (int j = 0; j < num-1; j++){
									queue_list.push(pre_order);
								}
							}
						}
						
						
						show_str += "\n";
						
						pre_is_num = is_num;
						is_num = false;
					}
				}
				else{
					if (!pre_is_num && !is_start && (in_commend.front() <= '3' && in_commend.front() >='0')){
						show_str += " # 1\n";
					}
					pre_is_num = is_num;
					
					switch(in_commend.front()){
						case '0':	// Order Water
							show_str += "Water";
							queue_list.push('0');
							pre_order = '0';
							is_start = false;
							break;
						case '1':	// Order Subway
							show_str += "Subway";
							queue_list.push('1');
							pre_order = '1';
							is_start = false;
							break;
						case '2':	// Order Coffee
							show_str += "Coffee";
							queue_list.push('2');
							pre_order = '2';
							is_start = false;
							break;
						case '3':	// Order Sandwich
							show_str += "Sandwich";
							queue_list.push('3');
							pre_order = '3';
							is_start = false;
							break;
						case '#':	// number
							if (is_start == false)
								show_str += " # ";
								pre_is_num = is_num;
								is_num = true;
							break;
						case 'D':	// Confirm order
							if (is_start == false){
								pthread_mutex_lock(&Order_List_lock);
								for (int j = 0; j < queue_list.size(); j++){
									Order_recv_List.push(queue_list.front());
									queue_list.pop();
								}
								pthread_mutex_unlock(&Order_List_lock);
							}
							show_str = "";
							queue_list = queue<char>();
							pre_is_num = is_num;
							is_num = false;
							is_start = true;
							pre_order = 0;
							break;
					}
				}
			}
			in_commend.pop();
		}
		
		pthread_mutex_lock(&POS_send_List_lock);
		POS_send_List = show_str;
		pthread_mutex_unlock(&POS_send_List_lock);
	}
	
	pthread_exit(NULL);
	
}