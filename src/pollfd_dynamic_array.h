#ifndef _POLLFD_DYNAMIC_ARRAY_H_
#define _POLLFD_DYNAMIC_ARRAY_H_

#include <poll.h>

#define POLLFD_DYNAMIC_ARRAY_OUTPUT_SIZE 8192

typedef struct PollFD_Dynamic_Array{
	struct pollfd* array;
	unsigned int count;
	unsigned int size;
	unsigned int growth_factor;
} PollFD_Dynamic_Array;

PollFD_Dynamic_Array* PollFD_Dynamic_Array_Create(unsigned int init_size);
void PollFD_Dynamic_Array_Insert(PollFD_Dynamic_Array* d_array, struct pollfd* fd_struct);
void PollFD_Dynamic_Array_Remove(PollFD_Dynamic_Array* d_array, unsigned int index);
char* PollFD_Dynamic_Array_String(PollFD_Dynamic_Array* d_array);
void PollFD_Dynamic_Array_Print(PollFD_Dynamic_Array* d_array);
void PollFD_Dynamic_Array_Destroy(PollFD_Dynamic_Array* d_array);

#endif
