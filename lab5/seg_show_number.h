unsigned long code[12] = {
0x3f,
0x6,
0x5b,
0x4f,
0x66,
0x6d,
0x7d,
0x7,
0x7f,
0x6f,
0x08,
0x77} ;

unsigned long seg_data_gen(int rule_num){
	unsigned long data = 0x0808 ; //00
	int dig10 = (rule_num/10)%10 ;
	int dig1 = rule_num%10 ;
	if(dig10 == 0){
		data = (data<<8) + 0x08 ;
	}
	else{
		data = (data<<8) + code[(rule_num/10)%10] ;
	}
	data = (data<<8) + code[dig1] ;
	return data ;
}
