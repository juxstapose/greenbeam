#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "pack.h"

void Binary_Reverse_Bytes(unsigned char* data, unsigned int size) {

	char temp = 0;
	int index = 0;
	int reverse_index = size-1;
	for(index = 0; index - reverse_index < 0; //stop when index and reverse index cross eachother 
		       index++, 
		       reverse_index--) {
		temp = *(data + index);
		*(data + index) = *(data + reverse_index);
		*(data + reverse_index) = temp;
	}
}

int Binary_Is_Int(char input) {
	if(input > 47 && input < 58) {
		return 1;
	}
	return 0;
}

unsigned char* Binary_Pack(char* format, ...) {
	va_list args;
	va_start(args, format);
	
	int total_size = Binary_Calcsize(format);
	unsigned char* data = (unsigned char*)malloc(total_size);

	char byte_order_type = *format;
	if(byte_order_type == '!' || byte_order_type == '@') {
		format++;
	}

	int size = 0;
	int index = 0;
	int prefix_index = 0;
	char number_prefix[10];
	memset(number_prefix, '\0', 10);
	*number_prefix = '1';
	int number = 0;
	while(*format != 0) {
		
		
		//if type is int
		char type;
		if(Binary_Is_Int(*format) == 1) {
			number_prefix[prefix_index] = *format;
			prefix_index++;	
		} else {
			type = *format;
			number = atoi(number_prefix);
			memset(number_prefix, '\0', 10);
			*number_prefix = '1';
			prefix_index = 0;
			size = Binary_Type_Count(type);
			int i = 0;
			if(type == 's') {
				char* in_string = (char*)va_arg(args, char*);
				memcpy(data+index, in_string, number);

			}
			for(i=0; i<number; i++) {
				if(type == 'b' || type == 'c') {
					char in_byte = (char)va_arg(args, int);
					memcpy(data+index, &in_byte, sizeof(char));
				}
				else if(type == 'B') {
					unsigned char in_ubyte = (unsigned char)va_arg(args, int);
					memcpy(data+index, &in_ubyte, sizeof(unsigned char));	
				}
				else if(type == 'i') {
					int in_int = va_arg(args, int);
					if(Binary_Is_Little_Endian() == 1 && byte_order_type == '!') {
						unsigned char* temp = (unsigned char*)malloc(sizeof(int));
						memcpy(temp, &in_int, sizeof(int));
						//reverse byte order to big endian for network
						Binary_Reverse_Bytes(temp, sizeof(int));
						memcpy(data+index, temp, sizeof(int));
						free(temp);
					} else {
						memcpy(data+index, &in_int, sizeof(int));	
					}
				}
				else if(type == 'I') {
					unsigned int in_uint = (unsigned int)va_arg(args, unsigned int);
					if(Binary_Is_Little_Endian() == 1 && byte_order_type == '!') {
						unsigned char* temp = (unsigned char*)malloc(sizeof(unsigned int));
						memcpy(temp, &in_uint, sizeof(unsigned int));
						//reverse byte order to big endian for network
						Binary_Reverse_Bytes(temp, sizeof(unsigned int));
						memcpy(data+index, temp, sizeof(unsigned int));
						free(temp);
					} else {
						memcpy(data+index, &in_uint, sizeof(unsigned int));	
					}
				}
				else if(type == 'h') {
					short in_short = (short)va_arg(args, int);
					if(Binary_Is_Little_Endian() == 1 && byte_order_type == '!') {
						unsigned char* temp = (unsigned char*)malloc(sizeof(short));
						memcpy(temp, &in_short, sizeof(short));
						//reverse byte order to big endian for network
						Binary_Reverse_Bytes(temp, sizeof(short));
						memcpy(data+index, temp, sizeof(short));
						free(temp);
					} else {
						memcpy(data+index, &in_short, sizeof(short));	
					}
				}
				else if(type == 'H') {
					unsigned short in_ushort = (unsigned short)va_arg(args, int);
					if(Binary_Is_Little_Endian() == 1 && byte_order_type == '!') {
						unsigned char* temp = (unsigned char*)malloc(sizeof(unsigned short));
						memcpy(temp, &in_ushort, sizeof(unsigned short));
						//reverse byte order to big endian for network
						Binary_Reverse_Bytes(temp, sizeof(unsigned short));
						memcpy(data+index, temp, sizeof(unsigned short));
						free(temp);
					} else {
						memcpy(data+index, &in_ushort, sizeof(unsigned short));	
					}
				}
				else if(type == 'l') {
					long in_long = (long)va_arg(args, long);
					if(Binary_Is_Little_Endian() == 1 && byte_order_type == '!') {
						unsigned char* temp = (unsigned char*)malloc(sizeof(long));
						memcpy(temp, &in_long, sizeof(long));
						//reverse byte order to big endian for network
						Binary_Reverse_Bytes(temp, sizeof(long));
						memcpy(data+index, temp, sizeof(long));
						free(temp);
					} else {
						memcpy(data+index, &in_long, sizeof(long));	
					}
				}	
				else if(type == 'L') {
					unsigned long in_ulong = (unsigned long)va_arg(args, unsigned long);
					if(Binary_Is_Little_Endian() == 1 && byte_order_type == '!') {
						unsigned char* temp = (unsigned char*)malloc(sizeof(unsigned long));
						memcpy(temp, &in_ulong, sizeof(unsigned long));
						//reverse byte order to big endian for network
						Binary_Reverse_Bytes(temp, sizeof(unsigned long));
						memcpy(data+index, temp, sizeof(unsigned long));
						free(temp);
					} else {
						memcpy(data+index, &in_ulong, sizeof(unsigned long));	
					}
				} 
				else if(type == 'f') {
					float in_float = (float)va_arg(args, double);
					if(Binary_Is_Little_Endian() == 1 && byte_order_type == '!') {
						unsigned char* temp = (unsigned char*)malloc(sizeof(float));
						memcpy(temp, &in_float, sizeof(float));
						//reverse byte order to big endian for network
						Binary_Reverse_Bytes(temp, sizeof(float));
						memcpy(data+index, temp, sizeof(float));
						free(temp);
					} else {
						memcpy(data+index, &in_float, sizeof(float));	
					}
				}
				else if(type == 'd') {
					double in_double = (double)va_arg(args, double);
					if(Binary_Is_Little_Endian() == 1 && byte_order_type == '!') {
						unsigned char* temp = (unsigned char*)malloc(sizeof(double));
						memcpy(temp, &in_double, sizeof(double));
						//reverse byte order to big endian for network
						Binary_Reverse_Bytes(temp, sizeof(double));
						memcpy(data+index, temp, sizeof(double));
						free(temp);
					} else {
						memcpy(data+index, &in_double, sizeof(double));	
					}
				}
				index += size;
			}
		}
		//then store it in number prefix
		//when we hit a character we atoi number prefix and use that for our prefix
		//then reset number prefix index to 0		
		format++;
	}
	
	va_end(args);
	return data;
}

//need to use variable arguments as pointers
void Binary_Unpack(char* format, unsigned char *data, ...) {
	va_list args;
	va_start(args, data);
	
	int total_size = Binary_Calcsize(format);
	unsigned char* temp = (unsigned char*)malloc(total_size);
	memcpy(temp, data, total_size);

	char byte_order_type = *format;
	if(byte_order_type == '!' || byte_order_type == '@') {
		format++;
	}

	int size = 0;
	int index = 0;
	int prefix_index = 0;
	char number_prefix[10];
	memset(number_prefix, '\0', 10);
	*number_prefix = '1';
	int number = 0;
	while(*format != 0) {
		char type;
		if(Binary_Is_Int(*format) == 1) {
			number_prefix[prefix_index] = *format;
			prefix_index++;	
		} else {
			type = *format;
			number = atoi(number_prefix);
			memset(number_prefix, '\0', 10);
			*number_prefix = '1';
			prefix_index = 0;
			size = Binary_Type_Count(type);
			int i = 0;
			//printf("type:%c size: %i num:%i index:%i\n", type, size, number, index);
			if(type == 's') {
				char* out_string = (char*)va_arg(args, char*);
				//printf("before memcpy\n");
				memcpy(out_string, temp+index, number);
				//printf("after memcpy\n");
			}
			for(i = 0; i<number; i++) {
				if(type == 'b' || type == 'c') {
					char* byte = (char*)va_arg(args, int*);
					memcpy(byte, temp+index, sizeof(char));
				}
				else if(type == 'B') {
					unsigned char* ubyte = (unsigned char*)va_arg(args, int*);
					memcpy(ubyte, temp+index, sizeof(unsigned char));	
				}
				else if(type == 'i') {
					int* out_int = (int*)va_arg(args, int*);
					if(Binary_Is_Little_Endian() == 1 && byte_order_type == '!') {
						//reverse byte order to big endian for network
						Binary_Reverse_Bytes(temp+index, sizeof(int));
					}
					memcpy(out_int, temp+index,  sizeof(int));	
				}
				else if(type == 'I') {
					unsigned int *out_uint = (unsigned int*)va_arg(args, int*);
					if(Binary_Is_Little_Endian() == 1 && byte_order_type == '!') {
						//reverse byte order to big endian for network
						Binary_Reverse_Bytes(temp+index, sizeof(unsigned int));
					}
					memcpy(out_uint, temp+index, sizeof(unsigned int));	
				}
				else if(type == 'h') {
					short* out_short = (short*)va_arg(args, int*);
					if(Binary_Is_Little_Endian() == 1 && byte_order_type == '!') {
						//reverse byte order to big endian for network
						Binary_Reverse_Bytes(temp+index, sizeof(short));
					}
					memcpy(out_short, temp+index, sizeof(short));	
				}
				else if(type == 'H') {
					unsigned short* out_ushort = (unsigned short*)va_arg(args, int*);
					if(Binary_Is_Little_Endian() == 1 && byte_order_type == '!') {
						//reverse byte order to big endian for network
						Binary_Reverse_Bytes(temp+index, sizeof(unsigned short));
					}
					memcpy(out_ushort, temp+index, sizeof(unsigned short));	
				}
				else if(type == 'l') {
					long* out_long = (long*)va_arg(args, long*);
					if(Binary_Is_Little_Endian() == 1 && byte_order_type == '!') {
						//reverse byte order to big endian for network
						Binary_Reverse_Bytes(temp+index, sizeof(long));
					}
					memcpy(out_long, temp+index, sizeof(long));	
				}	
				else if(type == 'L') {
					unsigned long* out_ulong = (unsigned long*)va_arg(args, long*);
					if(Binary_Is_Little_Endian() == 1 && byte_order_type == '!') {
						//reverse byte order to big endian for network
						Binary_Reverse_Bytes(temp+index, sizeof(unsigned long));
					}
					memcpy(out_ulong, temp+index, sizeof(unsigned long));	
				} 
				else if(type == 'f') {
					float* out_float = (float*)va_arg(args, float*);
					if(Binary_Is_Little_Endian() == 1 && byte_order_type == '!') {
						//reverse byte order to big endian for network
						Binary_Reverse_Bytes(temp+index, sizeof(float));
					}
					memcpy(out_float, temp+index, sizeof(float));
				}
				else if(type == 'd') {
					double* out_double = (double*)va_arg(args, double*);
					if(Binary_Is_Little_Endian() == 1 && byte_order_type == '!') {
						//reverse byte order to big endian for network
						Binary_Reverse_Bytes(temp+index, sizeof(double));
					}
					memcpy(out_double, temp+index, sizeof(double));
				}
				index += size;
			}
		}
		format++;
	}
	va_end(args);
}



int Binary_Calcsize(char* format) {
	int result = 0;
	if(*format == '!' || *format == '@') {
		format++;	
	}
	int size = 0;
	int prefix_index = 0;
	char number_prefix[10];
	memset(number_prefix, '\0', 10);
	*number_prefix = '1';
	int number = 0;
	while(*format != 0) {
		char type;
		if(Binary_Is_Int(*format) == 1) {
			number_prefix[prefix_index] = *format;
			prefix_index++;	
		} else {
			type = *format;
			number = atoi(number_prefix);
			memset(number_prefix, '\0', 10);
			*number_prefix = '1';
			prefix_index = 0;
			size = 0;
			int i = 0;
			for(i=0;i<number; i++) {
				size = size + Binary_Type_Count(type);
			}
		       result = result + size;
		}	
		
		format++;
	}
	return result;
}

int Binary_Type_Count(char type) {
	if(type == 'b' || type == 'c') {
		return sizeof(char);
	}
	if(type == 'B') {
		return sizeof(unsigned char);
	}
	if(type == 'i') {
		return sizeof(int);
	}
	if(type == 'I') {
		return sizeof(unsigned int);
	}
	if(type == 'h') {
		return sizeof(short);
	}
	if(type == 'H') {
		return sizeof(unsigned short);
	}
	if(type == 'l') {
		return sizeof(long);
	}	
	if(type == 'L') {
		return sizeof(unsigned long);
	}
	if(type == 'f') {
		return sizeof(float);
	}
	if(type == 'd') {
		return sizeof(double);
	}
	if(type == 's') {
		return sizeof(char);
	}
	return 0;
}

int Binary_Is_Little_Endian() {

	short int num = 0x1;
	char* num_ptr = (char*)&num;
	return (num_ptr[0] == 1);

}

char* Binary_String(char* format, unsigned char* data) {
	
	int size = 0;
	int format_index = 0;
	int index = 0;	
	int total_size = Binary_Calcsize(format);

	char byte_order_type = *format;
	if(byte_order_type == '!' || byte_order_type == '@') {
		format++;
	}
	
	unsigned char* temp = (unsigned char*)malloc(total_size);
	memset(temp, 0, total_size);
	memcpy(temp, data, total_size);
	
	int prefix_index = 0;
	char number_prefix[10];
	memset(number_prefix, '\0', 10);
	*number_prefix = '1';
	int number = 0;
	int length = strlen(format);
	char* result = (char*)malloc(PACK_MAX_BUFFER_SIZE);
	memset(result, 0, PACK_MAX_BUFFER_SIZE);
	int total_bytes = 0;
	int bytes = 0;
	while(*format != 0) {
		char type;
		//printf("type: %c total_bytes: %i\n", *format, total_bytes);
		if(format_index == 0) {
			bytes = sprintf(result, "%s", "[");
			result += bytes;
			total_bytes += bytes;
		}
		if(Binary_Is_Int(*format) == 1) {
			number_prefix[prefix_index] = *format;
			prefix_index++;	
		} else {
			type = *format;
			number = atoi(number_prefix);
			memset(number_prefix, '\0', 10);
			*number_prefix = '1';
			prefix_index = 0;
			size = Binary_Type_Count(type);
			int i;
			//printf("type: %c number: %i\n", type, number);
			//printf("total bytes:%i\n", total_bytes);
			if(type == 's') {
				memcpy(result, temp+index, number-1);
				bytes = number-1;
				result += bytes;
				total_bytes += bytes;
						
				if( !(format_index == length-1) ) {
					bytes = sprintf(result, " ");
					//printf("bytes written for space: %i\n", bytes);
					result+=bytes;
					total_bytes += bytes;
				}
				index += number;
				
			} else {
				//printf("format_index: %i length-1:%i\n", format_index, length-1);
				for(i = 0; i<number; i++) {
					if(type == 'c') {
						char value;
						if(Binary_Is_Little_Endian() == 1 && byte_order_type == '!') {
							//reverse byte order to big endian for network
							Binary_Reverse_Bytes(temp+index, size);
						}
						memcpy(&value, temp+index, size);
						//printf("int value: %i\n", value);
						
						bytes = sprintf(result, "%hhi", value);
						//printf("bytes written for int: %i\n", bytes);
						result+=bytes;
						total_bytes += bytes;
							
						if( !(format_index == length-1 && i == number-1) ) {
							bytes = sprintf(result, " ");
							//printf("bytes written for space: %i\n", bytes);
							result+=bytes;
							total_bytes += bytes;
						}
						index += size;
					} else if(type == 'b') {
						char value;
						if(Binary_Is_Little_Endian() == 1 && byte_order_type == '!') {
							//reverse byte order to big endian for network
							Binary_Reverse_Bytes(temp+index, size);
						}
						memcpy(&value, temp+index, size);
						//printf("int value: %i\n", value);
						
						bytes = sprintf(result, "%hhi", value);
						//printf("bytes written for int: %i\n", bytes);
						result+=bytes;
						total_bytes += bytes;
						
						if( !(format_index == length-1 && i == number-1) ) {
							bytes = sprintf(result, " ");
							//printf("bytes written for space: %i\n", bytes);
							result+=bytes;
							total_bytes += bytes;
						}
						index += size;
					} else if (type == 'B') {
						unsigned char value;
						if(Binary_Is_Little_Endian() == 1 && byte_order_type == '!') {
							//reverse byte order to big endian for network
							Binary_Reverse_Bytes(temp+index, size);
						}
						memcpy(&value, temp+index, size);
						//printf("int value: %i\n", value);
						
						bytes = sprintf(result, "%hhu", value);
						//printf("bytes written for int: %i\n", bytes);
						result+=bytes;
						total_bytes += bytes;
						
						if( !(format_index == length-1 && i == number-1) ) {
							bytes = sprintf(result, " ");
							//printf("bytes written for space: %i\n", bytes);
							result+=bytes;
							total_bytes += bytes;
						}	
						index += size;
					} else if (type == 'i') {
						int value;
						if(Binary_Is_Little_Endian() == 1 && byte_order_type == '!') {
							//reverse byte order to big endian for network
							Binary_Reverse_Bytes(temp+index, size);
						}
						memcpy(&value, temp+index, size);
						//printf("int value: %i\n", value);
						
						bytes = sprintf(result, "%i", value);
						//printf("bytes written for int: %i\n", bytes);
						result+=bytes;
						total_bytes += bytes;
						
						if( !(format_index == length-1 && i == number-1) ) {
							//printf("i:%i %i val:%i\n", i, number-1, value);	
							bytes = sprintf(result, " ");
							//printf("bytes written for space: %i\n", bytes);
							result+=bytes;
							total_bytes += bytes;
						}
						index += size;
					} else if(type == 'I') {
						unsigned int value;
						if(Binary_Is_Little_Endian() == 1 && byte_order_type == '!') {
							//reverse byte order to big endian for network
							Binary_Reverse_Bytes(temp+index, size);
						}
						memcpy(&value, temp+index, size);
						//printf("int value: %i\n", value);
						
						bytes = sprintf(result, "%u", value);
						//printf("bytes written for int: %i\n", bytes);
						result+=bytes;
						total_bytes += bytes;
						
						if( !(format_index == length-1 && i == number-1) ) {
							bytes = sprintf(result, " ");
							//printf("bytes written for space: %i\n", bytes);
							result+=bytes;
							total_bytes += bytes;
						}	
						index += size;
					} else if(type == 'h') {
						short value;
						if(Binary_Is_Little_Endian() == 1 && byte_order_type == '!') {
							//reverse byte order to big endian for network
							Binary_Reverse_Bytes(temp+index, size);
						}
						memcpy(&value, temp+index, size);
						//printf("int value: %i\n", value);
						
						bytes = sprintf(result, "%hi", value);
						//printf("bytes written for int: %i\n", bytes);
						result+=bytes;
						total_bytes += bytes;
						
						if( !(format_index == length-1 && i == number-1) ) {
							bytes = sprintf(result, " ");
							//printf("bytes written for space: %i\n", bytes);
							result+=bytes;
							total_bytes += bytes;
						}
						index += size;
					} else if(type == 'H') {
						unsigned short value;
						if(Binary_Is_Little_Endian() == 1 && byte_order_type == '!') {
							//reverse byte order to big endian for network
							Binary_Reverse_Bytes(temp+index, size);
						}
						memcpy(&value, temp+index, size);
						//printf("int value: %i\n", value);
						
						bytes = sprintf(result, "%hu", value);
						//printf("bytes written for int: %i\n", bytes);
						result+=bytes;
						total_bytes += bytes;
						
						if( !(format_index == length-1 && i == number-1) ) {
							bytes = sprintf(result, " ");
							//printf("bytes written for space: %i\n", bytes);
							result+=bytes;
							total_bytes += bytes;
						}	
						index += size;
					} else if(type == 'l') {
						long value;
						if(Binary_Is_Little_Endian() == 1 && byte_order_type == '!') {
							//reverse byte order to big endian for network
							Binary_Reverse_Bytes(temp+index, size);
						}
						memcpy(&value, temp+index, size);
						//printf("int value: %i\n", value);
						
						bytes = sprintf(result, "%li", value);
						//printf("bytes written for int: %i\n", bytes);
						result+=bytes;
						total_bytes += bytes;
						
						if( !(format_index == length-1 && i == number-1) ) {
							bytes = sprintf(result, " ");
							//printf("bytes written for space: %i\n", bytes);
							result+=bytes;
							total_bytes += bytes;
						}
						index += size;
					} else if(type == 'L') {
						unsigned long value;
						if(Binary_Is_Little_Endian() == 1 && byte_order_type == '!') {
							//reverse byte order to big endian for network
							Binary_Reverse_Bytes(temp+index, size);
						}
						memcpy(&value, temp+index, size);
						//printf("int value: %i\n", value);
						
						bytes = sprintf(result, "%lu", value);
						//printf("bytes written for int: %i\n", bytes);
						result+=bytes;
						total_bytes += bytes;
						
						if( !(format_index == length-1 && i == number-1) ) {
							bytes = sprintf(result, " ");
							//printf("bytes written for space: %i\n", bytes);
							result+=bytes;
							total_bytes += bytes;
						}
						index += size;
					} else if(type == 'f') {
						float value;
						if(Binary_Is_Little_Endian() == 1 && byte_order_type == '!') {
							//reverse byte order to big endian for network
							Binary_Reverse_Bytes(temp+index, size);
						}
						memcpy(&value, temp+index, size);
						//printf("int value: %i\n", value);
						
						bytes = sprintf(result, "%f", value);
						//printf("bytes written for int: %i\n", bytes);
						result+=bytes;
						total_bytes += bytes;
						
						if( !(format_index == length-1 && i == number-1) ) {
							bytes = sprintf(result, " ");
							//printf("bytes written for space: %i\n", bytes);
							result+=bytes;
							total_bytes += bytes;
						}
						index += size;
					} else if(type == 'd') {
						double value;
						if(Binary_Is_Little_Endian() == 1 && byte_order_type == '!') {
							//reverse byte order to big endian for network
							Binary_Reverse_Bytes(temp+index, size);
						}
						memcpy(&value, temp+index, size);
						//printf("int value: %i\n", value);
						
						bytes = sprintf(result, "%f", value);
						//printf("bytes written for int: %i\n", bytes);
						result+=bytes;
						total_bytes += bytes;
						
						if( !(format_index == length-1 && i == number-1) ) {
							bytes = sprintf(result, " ");
							//printf("bytes written for space: %i\n", bytes);
							result+=bytes;
							total_bytes += bytes;
						}
						index += size;
					}

				}//end for
			}//end else
		}//end if
		if(format_index == length-1) {
			bytes = sprintf(result, "%s", "]");
			result += bytes;
			total_bytes += bytes;
		}
		format_index++; //next character index
		format++;
	}//end while
	result -= total_bytes;	
	return result;
}

void Binary_Print(char* format, unsigned char* data) { 
	char* result = Binary_String(format, data);
	printf("%s\n", result);
	free(result);
}
