#ifndef _UTIL_H_
#define _UTIL_H_

#define SHA256_DIGEST_LENGTH 32
#define SHA_STRING_SIZE 65

#include <inttypes.h>

unsigned int Util_Count_Digits(int n);
unsigned int Util_Count_Digits64(uint64_t n);
void Util_Remove_All_Chars(char* str, char c);
void Util_Sha256(char *string, char outputBuffer[SHA_STRING_SIZE]);
char* Util_Formatted_Timestamp();
uint64_t Util_Microsecond_Timestamp();

#endif
