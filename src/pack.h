#ifndef _PACK_H_
#define _PACK_H_

#define PACK_MAX_BUFFER_SIZE 8192

void Binary_Reverse_Bytes(unsigned char* bytes, unsigned int size);
void Binary_Test_Reverse_Bytes();
unsigned char* Binary_Pack(char* format, ...);
void Binary_Unpack(char* format, unsigned char *bytes, ...);
char* Binary_String(char* format, unsigned char* data);
void Binary_Print(char* format, unsigned char* bytes);
int Binary_Calcsize(char* format);
int Binary_Type_Count(char type);
int Binary_Is_Little_Endian();
void Binary_Test_Calcsize();
void Binary_Test_UnpackPack();

#endif
