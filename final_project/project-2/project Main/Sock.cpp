#include "Sock.hpp"

pthread_mutex_t vec_send_lock;
queue<string> vec_send_str;
pthread_mutex_t vec_recv_lock;
queue<string> vec_recv_str;

int sock_type_idx = -1;

void* ClientHandler(void *thread_in){
	pthread_detach(pthread_self()); 

	THREAD_OBJ* thr_obj = (THREAD_OBJ*) thread_in;
	int connfd = thr_obj->connfd;
	free(thr_obj);
	
	int n;
	char snd[BUFSIZE], rcv[BUFSIZE];

	memset(rcv, 0, BUFSIZE);
	memset(snd, 0, BUFSIZE);
/* Step1 Handshake ID */
	// read message from client
	if ((n = read(connfd, rcv, BUFSIZE)) < 0)
		errexit("Error: read ID\n");

	// write message back to the client
	string snd_str(rcv);

	if ((n = write(connfd, snd_str.c_str(), n)) < 0)
		errexit("Error: write back ID\n");
	
	if (snd_str.compare("MC") == 0){
		cout << "MC online" << endl;
		sock_type_idx = 1;
	}
	else if (snd_str.compare("WL") == 0){
		cout << "WL online" << endl;
		sock_type_idx = 2;
	}
	else if (snd_str.compare("POS") == 0){
		cout << "POS online" << endl;
		sock_type_idx = 3;
	}
	else{
		cout << "ID: " << snd_str << " not recognized!" << endl;
		pthread_exit(NULL);
	}

/* Step2 I/O Control */
	char buf[BUFSIZE];

	
	while(1){
		usleep(WAIT_TIME);
		n = read(connfd, buf, BUFSIZE);
		if (n <= 0){
			cout << n << " recv Disconnected\n" << endl;
			break;
		}
		
		string recv_str(buf);
		if (sock_type_idx == 1){
			pthread_mutex_lock(&MC_recv_List_lock);
			for (int i = 0; i < recv_str.length(); i++){
				if (recv_str[i] != 'e')
					MC_recv_List.push(recv_str[i]);
			}
			pthread_mutex_unlock(&MC_recv_List_lock);
		}
		else if (sock_type_idx == 2){
			pthread_mutex_lock(&WL_recv_List_lock);
			for (int i = 0; i < recv_str.length(); i++){
				if (recv_str[i] != 'e')
					WL_recv_List.push(recv_str[i]);
			}
			pthread_mutex_lock(&WL_recv_List_lock);
		}
		else if (sock_type_idx == 3){
			pthread_mutex_lock(&POS_recv_List_lock);
			for (int i = 0; i < recv_str.length(); i++){
				if (recv_str[i] != 'e')
					POS_recv_List.push(recv_str[i]);
			}
			pthread_mutex_lock(&POS_recv_List_lock);
		}
		
		usleep(WAIT_TIME);
		
		
		
		string send_str("");
		
		if (sock_type_idx == 1){
			pthread_mutex_lock(&MC_recv_List_lock);
			send_str = MC_send_List;
			pthread_mutex_unlock(&MC_recv_List_lock);
		}
		else if (sock_type_idx == 2){
			pthread_mutex_lock(&WL_recv_List_lock);
			send_str = WL_send_List;
			pthread_mutex_lock(&WL_recv_List_lock);
		}
		else if (sock_type_idx == 3){
			pthread_mutex_lock(&POS_recv_List_lock);
			send_str = POS_send_List;
			pthread_mutex_lock(&POS_recv_List_lock);
		}
		
		if (send_str.size() == 0){
			send_str = "e";
		}
		
		n = write(connfd, send_str.c_str(), send_str.length());
		if (n <= 0){
			cout << n << " send Disconnected\n" << endl;
			break;
		}
	}

	
	
	close(connfd);
	
	pthread_exit(NULL);
}
