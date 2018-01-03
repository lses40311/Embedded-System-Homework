#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <string.h>
#include <iostream>
#include <map>
#include <vector>


#define FILENAME "orders.txt"
#define LIST_MAX 100

using namespace std ;

int main(){
    FILE * file ;
    char * buf ;
    map<string, string> waiting_comfirm_list;

    // open file
    file = fopen(FILENAME, "r") ;
    buf = new char[30] ;

    while(1){
        if(fscanf(file, "%s\n", buf) > 0){
            string s = buf ;
            char * pch = strrchr(buf, ',')                                                                          ;
            cout << s << endl ;
            cout << "Pin number = " << (int)strlen(pch) << " " << pch+1 << endl ;
            waiting_comfirm_list[pch+1] = s ;
        }
    }

    fclose(file) ;
    return 0 ;
}
