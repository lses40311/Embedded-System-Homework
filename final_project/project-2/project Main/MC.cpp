#include "MC.hpp"

char get_food_id(string s){
	cout << "finding food id: " << s << endl ;
	if(s == "water"){
		return '0' ;
	}
	else if(s == "subway"){
		return '1' ;
	}
	else if(s == "coffee"){
		return '2' ;
	}
	else if(s == "sandwich"){
		return '3' ;
	}
	else{
		printf("Not such food...\n");
		return '9' ;
	}
}

void *threadMobileCounter(void *threadid)
{
	FILE * file ;
	char * buf ;
	char * pin_buff ;
	map<string, string> waiting_comfirm_list;
	int ptr ;

	file = fopen(FILENAME, "r") ;
	buf = new char[BUF_SIZE] ;
	ptr = 0 ;
	pin_buff = new char[4] ;
	while (1){
			if(fscanf(file, "%s\n", buf) > 0){
				string s = buf ;
				int cnt = 0 ;
				int i = 2 ;
				char * pch = strtok(buf, ",") ;

				string key = pch ;
				cout << "Receive a new request from Chatbot: " << s << endl ;
				cout << "The Pin number is " << key << endl ;
				waiting_comfirm_list[key] = s ;
			}

			pthread_mutex_lock(&MC_recv_List_lock);
			for (int i = 0; i < MC_recv_List.size(); i++) {
				char c = MC_recv_List.front() ;
				pthread_mutex_lock(&MC_send_List_lock);
				MC_send_List = c ;
				pthread_mutex_unlock(&MC_send_List_lock);
				cout << c << endl ;
				if(c == '#'){
					cout << "Recv PIN: " << pin_buff << endl ;
					string order_str = waiting_comfirm_list[pin_buff] ;
					if(order_str != ""){
						cout << "Find an order with matched PIN." << endl ;
						cout << order_str << endl ;
						char arr[BUF_SIZE] ;
						strncpy(arr, order_str.c_str(), sizeof(arr));
						arr[sizeof(arr) - 1] = 0;
						string msg = ("Hi, ") ;
						char * tok = strtok(arr, ",") ;
						tok = strtok(NULL, ",") ;
						msg = msg + string(tok) + string(". You have ordered...\n");
						tok = strtok(NULL, ",") ;
						while(tok != NULL){
							msg = msg + string(tok) + string("\n") ;
							pthread_mutex_lock(&Order_List_lock);
							Order_recv_List.push(get_food_id(string(tok))) ;
							cout << "queue back -> " << Order_recv_List.back() << endl ;
							pthread_mutex_unlock(&Order_List_lock);
							tok = strtok(NULL, ",") ;
						}
						msg = msg + string("Please wait a few minutes...") ;
						pthread_mutex_lock(&MC_send_List_lock);
						MC_send_List = msg ;
						pthread_mutex_unlock(&MC_send_List_lock);
					}
					else {
						cout << "no match" << endl ;
						pthread_mutex_lock(&MC_send_List_lock);
						MC_send_List = "Wrong code, please enter again." ;
						pthread_mutex_unlock(&MC_send_List_lock);
					}
					ptr = 0 ;
					memset (pin_buff, 0, 4) ;
				}
				else{
					pin_buff[ptr] = c ;
					ptr++ ;
					if(ptr > 3){
						pthread_mutex_lock(&MC_send_List_lock);
						MC_send_List = "Too many digits, please enter again." ;
						pthread_mutex_unlock(&MC_send_List_lock);
						ptr = 0 ;
						memset (pin_buff, 0, 4) ;
					}
				}
				MC_recv_List.pop();
			}
			pthread_mutex_unlock(&MC_recv_List_lock);

	}
	fclose(file) ;
	pthread_exit(NULL);

}
