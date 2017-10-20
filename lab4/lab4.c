#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define PXA 1
#define MAX_POINT_NUM 81

#if PXA
#include "asm-arm/arch-pxa/lib/creator_pxa270_lcd.h"
#endif

// Function Declare
void print_cordinators() ;

int main(){
	char x[MAX_POINT_NUM] ;
	char y[MAX_POINT_NUM] ;
	
	#if PXA
	int lcd_fd ;
	if((lcd_fd = open("/dev/lcd", O_RDWR)) < 0){
		printf("open failed\n") ;
		exit(-1) ;
	}
	#endif

	print_cordinators(x,y, lcd_fd) ;
	return 0 ;
}

#if PXA
void print_cordinators(char * x, char * y, int lcd_fd){ // covert 9*9 to LCD
	
	unsigned short vals[4] ={0xffff, 0xfff0, 0xff00, 0xf000} ; 
	ioctl(lcd_fd, LCD_IOCTL_CLEAR, NULL) ;
	lcd_full_image_info_t image ;
	//image.data[0] = 0xffff ;
	int i = 0 ;
	


	//for(i=0; i< 225; i++){
	//	if(i%16==0) image.data[i] = 0xffff ;
	//	if(i%16==1) image.data[i] = 0x0fff ;
	//}
	// ioctl(lcd_fd, LCD_IOCTL_CLEAR, NULL) ;
	// display.Count = sprintf((char*) display.Msg, " %d", val) ;
	ioctl(lcd_fd, LCD_IOCTL_DRAW_FULL_IMAGE, &image) ;
}
#endif
