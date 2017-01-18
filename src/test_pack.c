#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "pack.h"

void Binary_Test_Reverse_Bytes() {
	
	int size = 4;
	unsigned char *data = (unsigned char*)malloc(size);
	*(data + 0) = 120; 
	*(data + 1) = 200;
	*(data + 2) = 43;
	*(data + 3) = 27;
	

	Binary_Reverse_Bytes(data, size);
	
	int index = 0;
	for(index=0; index<size; index++) {	
		printf("%i\n", *(data +index));
	}
		
	free(data);
	
	int size_two = 3;
	unsigned char *data_two = (unsigned char*)malloc(size_two);
	*(data_two + 0) = 100; 
	*(data_two + 1) = 25;
	*(data_two + 2) = 50;
	
	Binary_Reverse_Bytes(data_two, size);
	
	index = 0;
	for(index=0; index<size; index++) {
		printf("%i\n", *(data +index));
	}
	
	free(data_two);
}

void Binary_Test_Calcsize() {

	int size = 0;
	int expected = 0;

	size = Binary_Calcsize("!bHi");	
	expected = 7;
	if(size == expected) {
		printf("test 1 passed\n");
	}
	size = Binary_Calcsize("@bbbbHiI");	
	expected = 14;
	if(size == expected) {
		printf("test 2 passed\n");
	}
	size = Binary_Calcsize("lhhbbbb");	
	expected = 16;
	if(size == expected) {
		printf("test 3 passed\n");
	}

}



void Binary_Test_UnpackPack() {
	
	unsigned int in_one_size = 11;
	char* in_string_one = "juxstapose";
	unsigned int in_two_size = 6;
	char* in_string_two = "hello";

	unsigned char* data = (unsigned char*)Binary_Pack("!I11sI6s", in_one_size, in_string_one, in_two_size, in_string_two);	
	
	
	unsigned int out_one_size;
	unsigned char* out_string_one = (unsigned char*)malloc(in_one_size);
	unsigned int out_two_size;
	unsigned char* out_string_two = (unsigned char*)malloc(in_two_size);
	

	Binary_Unpack("!I11sI", data, &out_one_size, out_string_one, &out_two_size);
	Binary_Print("!I11sI", data);	

	Binary_Unpack("!I11sI6s", data, &out_one_size, out_string_one, &out_two_size, out_string_two);
	Binary_Print("!I11sI6s", data);	
		
	free(out_string_one);		
	free(out_string_two);		

	char in_byte = 127;	
	unsigned char in_uc = 255;
	int in_i = -4000;
	unsigned int in_ui = 4020;
	short in_s = -32768;
	unsigned short in_us = 42;
	long in_l = 1000000000;
	unsigned long in_ul = 2148000000;
	
	data = (unsigned char*)Binary_Pack("bBiIhHlL", in_byte, in_uc, in_i, in_ui, in_s, in_us, in_l, in_ul);
	
	char out_byte;
	unsigned char out_uc;
	int out_i;
	unsigned int out_ui;
	short out_s;
	unsigned short out_us;
	long out_l;
	unsigned long out_ul;	

	Binary_Unpack("bBiIhHlL", data, &out_byte, &out_uc, &out_i, &out_ui, &out_s, &out_us, &out_l, &out_ul);
	
	Binary_Print("bBiIhHlL", data);
	
	if(in_byte == out_byte) {
		printf("test 1 passed\n");
	}
	if(in_uc == out_uc) {
		printf("test 2 passed\n");
	}
	if(in_i == out_i) {
		printf("test 3 passed\n");
	}
	if(in_ui == out_ui) {
		printf("test 4 passed\n");
	}
	if(in_s == out_s) {
		printf("test 5 passed\n");
	}
	if(in_us == out_us) {
		printf("test 6 passed\n");
	}
	if(in_l == out_l) {
		printf("test 7 passed\n");
	}
	if(in_ul == out_ul) {
		printf("test 8 passed\n");
	}
	
	free(data);

	char in_byte_one = 123;
	char in_byte_two = 20;
	char in_byte_three = 45;
	
	data = (unsigned char*)Binary_Pack("!bbb", in_byte_one, in_byte_two, in_byte_three);

	char out_byte_one;
	char out_byte_two;
	char out_byte_three;
	
	Binary_Print("!bbb", data);
	Binary_Unpack("!bbb", data, &out_byte_one, &out_byte_two, &out_byte_three);
	
	if(in_byte_one == out_byte_one) {
		printf("test 9 passed\n");
	}
	if(in_byte_two == out_byte_two) {
		printf("test 10 passed\n");
	}
	if(in_byte_three == out_byte_three) {
		printf("test 11 passed\n");
	}
	
	free(data);
	
	in_byte_one = 20;
	in_byte_two = 92;
	in_byte_three = 73;
	char in_byte_four = 20;
	
	data = (unsigned char*)Binary_Pack("!bbbb", in_byte_one, in_byte_two, in_byte_three, in_byte_four);

	char out_byte_four;
	Binary_Print("!bbbb", data);
	
	Binary_Unpack("!bbbb", data, &out_byte_one, &out_byte_two, &out_byte_three, &out_byte_four);
	
	if(in_byte_one == out_byte_one) {
		printf("test 12 passed\n");
	}
	if(in_byte_two == out_byte_two) {
		printf("test 13 passed\n");
	}
	if(in_byte_three == out_byte_three) {
		printf("test 14 passed\n");
	}
	if(in_byte_four == out_byte_four) {
		printf("test 15 passed\n");
	}
	
	in_byte_one = 72;
	in_byte_two = 40;
	in_byte_three = 30;
	in_byte_four = 85;

	short in_s_one = 4000;	
	short in_s_two = 5000;	
	
	data = (unsigned char*)Binary_Pack("4b2h", in_byte_one, in_byte_two, in_byte_three, in_byte_four, in_s_one, in_s_two);
	

	short out_s_one;
	short out_s_two;

	Binary_Print("4b2h", data);
	Binary_Unpack("4b2h", data, &out_byte_one, &out_byte_two, &out_byte_three, &out_byte_four, &out_s_one, &out_s_two);
	
	if(in_byte_one == out_byte_one) {
		printf("test 16 passed\n");
	}
	if(in_byte_two == out_byte_two) {
		printf("test 17 passed\n");
	}
	if(in_byte_three == out_byte_three) {
		printf("test 18 passed\n");
	}
	if(in_byte_four == out_byte_four) {
		printf("test 19 passed\n");
	}
	if(in_s_one == out_s_one) {
		printf("test 20 passed\n");
	}
	if(in_s_two == out_s_two) {
		printf("test 21 passed\n");
	}
	
	float in_float_one = 7.45;
	double in_double_one = 9.442142849243;
	data = (unsigned char*)Binary_Pack("fd", in_float_one, in_double_one);
	
	float out_float_one;
	double out_double_one;
	
	printf("fd\n");	
	Binary_Print("fd", data);
	Binary_Unpack("fd", data, &out_float_one, &out_double_one);
	
	if(in_float_one == out_float_one) {
		printf("test 22 passed\n");
	}
	if(in_double_one == out_double_one) {
		printf("test 23 passed\n");
	}
	
	in_float_one = 8.88;
	float in_float_two = 23.48328;
	long in_long_one = 4309534;
	in_byte_one = 110;
	in_byte_two = 89;
	in_byte_three = 9;
	in_byte_four = 40;
	data = (unsigned char*)Binary_Pack("2fl4b", in_float_one, in_float_two, in_long_one, in_byte_one, in_byte_two, in_byte_three, in_byte_four);
	Binary_Print("2fl4b", data);
	float out_float_two;
	float out_long_one;
	Binary_Unpack("2fl4b", data, &out_float_one, &out_float_two, &out_long_one, &out_byte_one, &out_byte_two, &out_byte_three, &out_byte_four);

	int in_int_one = 5689;
	int in_int_two = 20;
	in_string_one = "Hello World";
	int in_int_three = 200;
	int in_int_four = 100;
	int in_int_five = 340;
	data = (unsigned char*)Binary_Pack("!2i12s3i", in_int_one, in_int_two, in_string_one, in_int_three, in_int_four, in_int_five);
	//Binary_Print("!2i12s3i", data);	
	
	int out_int_one;	
	int out_int_two;
	out_string_one = (char*)malloc(strlen(in_string_one)+1);
	memset(out_string_one, '\0', strlen(in_string_one)+1);	
	int out_int_three;	
	int out_int_four;	
	int out_int_five;		
	
	Binary_Unpack("!2i12s3i", data, &out_int_one, &out_int_two, out_string_one, &out_int_three, &out_int_four, &out_int_five);
	
	if(in_int_one == out_int_one) { 
		printf("test 24 passed\n");
	}
	if(in_int_two == out_int_two) { 
		printf("test 25 passed\n");
	}
	if(strcmp(in_string_one, out_string_one) == 0) { 
		printf("test 26 passed\n");
	}
	if(in_int_three == out_int_three) { 
		printf("test 27 passed\n");
	}
	if(in_int_four == out_int_four) { 
		printf("test 28 passed\n");
	}
	if(in_int_five == out_int_five) { 
		printf("test 29 passed\n");
	}

	Binary_Print("!2i12s3i", data);	

	free(out_string_one);
	free(data);
}
int main(int argc, char* argv[]) { 
	
	Binary_Test_UnpackPack();

	return 0;
}
