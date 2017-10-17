#include "asm-arm/arch-pxa/lib/creator_pxa270_lcd.h"
#include <stdlib.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

int main(){
    int fd;
    int retval ;
    unsigned short data ;

    if((fd = open("/dev/lcd", O_RDWR)) < 0){
        printf("open faled\n") ;
        exit(-1) ;
    }

    data=LED_ALL_ON ;
    ioctl (fd , LED_IOCTL_SET, &data );
    printf("Turn on all LED lamps\n");
    sleep (3);
	
	data=LED_ALL_OFF ;
	ioctl (fd , LED_IOCTL_SET, &data );
	printf("Turn off all LED lamps\n");
	sleep (3);

    close(fd) ;
	return 0 ;
}

