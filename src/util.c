#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h>
#include <inttypes.h>
#include <time.h>
#include <sys/time.h>
#include "util.h"

unsigned int Util_Count_Digits(int n) {
	int result = 0;
	while(n != 0) {
		n /= 10;
		++result;
	}
	return result;
}

unsigned int Util_Count_Digits64(uint64_t n) {
	unsigned int result = 0;
	while(n != 0) {
		n /= 10;
		++result;
	}
	return result;
}

void Util_Remove_All_Chars(char* str, char c) {
    char *pr = str, *pw = str;
    while (*pr) {
        *pw = *pr++;
        pw += (*pw != c);
    }
    *pw = '\0';
}

void Util_Sha256(char *string, char outputBuffer[SHA_STRING_SIZE]) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, string, strlen(string));
    SHA256_Final(hash, &sha256);
    int i = 0;
    for(i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        sprintf(outputBuffer + (i * 2), "%02x", hash[i]);
    }
    outputBuffer[64] = 0;
}

uint64_t Util_Microsecond_Timestamp() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec*(uint64_t)1000000+tv.tv_usec;	
}

char* Util_Formatted_Timestamp() {
	int bytes = 0;
	
	time_t rawtime;
	struct tm *info;

	time(&rawtime);
	info = localtime(&rawtime);
	int year = info->tm_year + 1900;	
	
	int mon = info->tm_mon + 1;
	char* month = (char*)malloc(3);
	memset(month, '\0', 3);
	if(mon < 10) {
		bytes = sprintf(month, "%i%i", 0, mon);
	} else {
		bytes = sprintf(month, "%i", mon);
	}
	char* day = (char*)malloc(3);
	memset(day, '\0', 3);
	if(info->tm_mday < 10) {
		bytes = sprintf(day, "%i%i", 0, info->tm_mday);
	} else {
		bytes = sprintf(day, "%i", info->tm_mday);
	}
	char* hour = (char*)malloc(3);
	memset(hour, '\0', 3);
	if(info->tm_hour < 10) {
		bytes = sprintf(hour, "%i%i", 0, info->tm_hour);
	} else {
		bytes = sprintf(hour, "%i", info->tm_hour);
	}
	char* min = (char*)malloc(3);
	memset(min, '\0', 3);
	if(info->tm_min < 10) {
		bytes = sprintf(min, "%i%i", 0, info->tm_min);
	} else {
		bytes = sprintf(min, "%i", info->tm_min);
	}
	char* sec = (char*)malloc(3);
	memset(sec, '\0', 3);
	if(info->tm_sec < 10) {
		bytes = sprintf(sec, "%i%i", 0, info->tm_sec);
	} else {
		bytes = sprintf(sec, "%i", info->tm_sec);
	}

	char* result = (char*)malloc(64);	
	bytes = sprintf(result, "%i-%s-%s %s:%s:%s", year,month, day, hour, min, sec);
	free(month);
	free(day);
	free(hour);
	free(min);
	free(sec);
	return result;
}


unsigned int Util_Comma_Delimited_Count(char* comma_delimited_names) {
	unsigned int count = 0;
	if(comma_delimited_names != NULL) {
		char* current = comma_delimited_names;
		while(*current != '\0') { 
			if(*current == ',') {
				count++;
			}
			current++;
		}
		return count + 1;
	}
	return count;
}

