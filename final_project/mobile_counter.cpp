#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <iostream>

#define FILENAME "orders.txt"

using namespace std ;

//char filename = "orders.txt" ;

int main(){
    int c;
    FILE *file;

    while(1){
        sleep(1) ;
        char * line= new char(100) ;
        size_t len ;
        
        if((file = fopen(FILENAME, "r"))){
            getline(&line, &len, file) ;
            printf("%s", line);
            fclose(file) ;
            remove(FILENAME) ;
        }
        else{
            printf("No file!\n") ;
        }
    }
    return 0 ;
}
