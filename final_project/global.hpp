#ifndef GLOBAL_HPP
#define GLOBAL_HPP

#include <queue>
#include <string>
using namespace std;

extern pthread_mutex_t MC_recv_List_lock;
extern pthread_mutex_t MC_send_List_lock;
extern queue<char> MC_recv_List;
extern string MC_send_List;

extern pthread_mutex_t WL_recv_List_lock;
extern pthread_mutex_t WL_send_List_lock;
extern queue<char> WL_recv_List;
extern string WL_send_List;

extern pthread_mutex_t POS_recv_List_lock;
extern pthread_mutex_t POS_send_List_lock;
extern queue<char> POS_recv_List;
extern string POS_send_List;


extern pthread_mutex_t Order_List_lock;
extern queue<char> Order_recv_List;


#endif