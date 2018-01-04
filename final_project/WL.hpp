#ifndef WL_HPP
#define WL_HPP

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string>
#include <iostream>
#include <vector>

#include <string.h> 
#include <signal.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <sys/time.h>

#include "global.hpp"
#include "project_header.h"


#define WL_WAIT_TIME 50000

using namespace std;


void *threadWaitingList(void *threadid);
void report_handler(int signum);

#endif