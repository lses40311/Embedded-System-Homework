#ifndef _SOCK_HPP
#define _SOCK_HPP

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <iostream>
#include "SockOp.h"
#include "global.hpp"
#include <queue>

using namespace std;

#define BUFSIZE 500
#define WAIT_TIME 100000


typedef struct thread_obj{
	int connfd;
} THREAD_OBJ;




void* ClientHandler(void *thread_in);


#endif