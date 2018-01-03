#ifndef MC_HPP
#define MC_HPP
#define FILENAME "orders.txt"
#define LIST_MAX 100
#define BUF_SIZE 1024


#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <iostream>
#include <unistd.h>
#include <string>
#include <string.h>
#include <map>
#include <vector>
#include <list>
#include "global.hpp"

using namespace std;


void *threadMobileCounter(void *threadid);


#endif
