#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int is_alph(char c){
	if(c >= 'a' && c<= 'z') return 1 ;
	else if(c >= 'A' && c<= 'Z') return 1 ;
	else if( c >= '0' && c<= '9') return 1 ;
	return 0 ;
}

// Remove the symbols at the beging & end of the string (not including the middle of the string)
char * remove_symbols(char * str, int len){
	char * s = (char*)malloc(sizeof(char)*len) ;
	int i, start, end ;
	start = end = 0 ;
	for(i=0; i<len ; i++){
		if(is_alph(str[i])){
			start = i ;
			break ;
		}
	}
	for(i=len; i>=0 ; i--){
		if(is_alph(str[i])){
			end = i ;
			break ;
		}
	}
	strncpy(s, &str[start], end - start +1) ;
	s[end - start + 1] = '\0' ;
	return s ;
}

int main(int argc, char ** argv){
	if(argc != 3){
		printf("usage: ./a [filename] [target word]\n") ;
		exit(0) ;
	}
	const char *filename, *target ;
	filename = argv[1] ;
	target = argv[2] ;

	FILE * fp ;
	if( (fp = fopen(filename, "r")) == NULL){
		printf("No such file!\n") ;
		exit(0) ;
	}
	
	char buff[256] ;
	char pre1[256] ;
	char pre2[256] ;
	char word[256] ;
	while(fscanf(fp, "%s", buff) == 1){
		int a = strcmp(remove_symbols(buff, strlen(buff)), target) ;
		// Found match 
		if(a == 0){
			printf("%s %s %s ", pre2, pre1, buff) ;
			int i ;
			for( i=0; i<2 ; i++){
				fscanf(fp, "%s", buff) ;
				printf("%s ", buff) ;
				memcpy(pre2, pre1, 256) ;
				memcpy(pre1, buff, 256) ;
			}
			// Comment the next 2 lines if u dont want to be track every time we find a match.
			printf("\nPlease hit ENTER to continue..\n") ;
			getchar() ;
		}
		// SWAP
		memcpy(pre2, pre1, 256) ; 
		memcpy(pre1, buff, 256) ;
	}

	return 0 ;
}
