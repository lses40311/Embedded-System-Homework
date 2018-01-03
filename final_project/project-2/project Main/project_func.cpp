/* 
 * project_func.c
 */

#include "project_header.h"

string getOrder(char order_index){

	switch(order_index){

		case '1':
			WL_send_List += "Subway\n";
			break;
		case '2':
			WL_send_List += "Coffee\n";
			break;
		case '3':
			WL_send_List += "Sandwich\n";
			break;
		default:
			break;

	}
}
