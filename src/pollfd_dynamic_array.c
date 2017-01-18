#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pollfd_dynamic_array.h"

PollFD_Dynamic_Array* PollFD_Dynamic_Array_Create(unsigned int init_size) {
	PollFD_Dynamic_Array* d_array = (PollFD_Dynamic_Array*)malloc(sizeof(PollFD_Dynamic_Array));
	d_array->array = calloc(init_size, sizeof(struct pollfd));
	d_array->count = 0;
	d_array->size = init_size;
	d_array->growth_factor = 2;
	return d_array;
}

void PollFD_Dynamic_Array_Insert(PollFD_Dynamic_Array* d_array, struct pollfd* fd_struct) {
	if(d_array->count == d_array->size) {
		d_array->size *= d_array->growth_factor;
		d_array->array = (struct pollfd*)realloc(d_array->array, d_array->size * sizeof(struct pollfd));
	}
	d_array->array[d_array->count++] = *fd_struct;

}

void PollFD_Dynamic_Array_Remove(PollFD_Dynamic_Array* d_array, unsigned int index) {
	if(d_array->size > 0) {
		struct pollfd* temp = calloc(d_array->size - 1, sizeof(struct pollfd));
		memcpy(temp, d_array->array, index);
		memcpy(temp+index, d_array->array + index + 1, d_array->size - index - 1);
		free(d_array->array);
		d_array->array = temp;
		d_array->size  -= 1;
		d_array->count -= 1;
	}

}
char* PollFD_Dynamic_Array_String(PollFD_Dynamic_Array* d_array) {
	char* result = (char*)malloc(POLLFD_DYNAMIC_ARRAY_OUTPUT_SIZE);
	memset(result, '\0', POLLFD_DYNAMIC_ARRAY_OUTPUT_SIZE);
	int i = 0;
	int bytes = 0;
	int total_bytes = 0;
	bytes = sprintf(result, "[");
	result+=bytes;
	total_bytes+=bytes;
	for(i=0; i<d_array->size; i++) {
		bytes = sprintf(result, "%i => %i", i, d_array->array[i].fd);
		result+=bytes;
		total_bytes+=bytes;
		if(i != d_array->size-1) {
			bytes = sprintf(result, "\n");
			result+=bytes;
			total_bytes+=bytes;
		}
	}
	bytes = sprintf(result, "]");
	result+=bytes;
	total_bytes+=bytes;
	result-=total_bytes;
	return result;
}

void PollFD_Dynamic_Array_Print(PollFD_Dynamic_Array* d_array) {
	char* output = PollFD_Dynamic_Array_String(d_array);
	printf("%s\n", output);
	free(output);
}

void PollFD_Dynamic_Array_Destroy(PollFD_Dynamic_Array* d_array) {
	if(d_array->array != NULL) {
		free(d_array->array);
	}
	d_array->size = 0;
	d_array->count = 0;
	d_array->array = NULL;
	if(d_array != NULL) {
		free(d_array);
	}
}

