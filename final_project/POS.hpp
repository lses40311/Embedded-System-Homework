#ifndef POS_HPP
#define POS_HPP

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <iostream>
#include <vector>
#include <string>
#include <unistd.h>

#include "global.hpp"


#define POS_WAIT_TIME 50000

using namespace std;

void *threadPOS(void *threadid);


#endif