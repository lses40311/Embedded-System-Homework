#ifndef WL_HPP
#define WL_HPP

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string>
#include <iostream>

#include "global.hpp"
#include "project_header.h"

using namespace std;


void *threadWaitingList(void *threadid);


#endif