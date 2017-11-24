#include <sys/sem.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

int main(int argc, char ** argv){
    int sem_num, s, i ;
    long int key ;
    key = (long int)atoi(argv[1]) ;
    sem_num = atoi(argv[2]) ;
    printf("%lu, %d\n", key, sem_num );
    s = semget(key, sem_num, 0) ;
    if(s < 0){
        printf("%s\n", strerror(errno));
        return -1 ;
    }
    for( i = 0; i < sem_num; i++){
        if(semctl(s, i, IPC_RMID, 0) < 0){
            printf("%s\n", "ERROR in removing sem");
            return -1 ;
        }
    }
    printf("%s\n", "Successfully removed.");
    return 0 ;
}
