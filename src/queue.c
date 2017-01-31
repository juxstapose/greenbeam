#include <stdio.h>
#include <stdlib.h>
#include "queue.h"
#include "pack.h"
#include "protocol.h"
#include "protocol_format.h"


Queue* Queue_Create() {
	Queue* queue = (Queue*)malloc(sizeof(Queue));
	queue->front = NULL;
	queue->rear = NULL;
	return queue;
}

void Queue_Enqueue(Queue* queue, unsigned char* data) {
	Queue_Node* temp = (Queue_Node*)malloc(sizeof(Queue_Node));
	temp->data = data; 
	temp->next = NULL;
	
	if(queue->front == NULL && queue->rear == NULL){
		queue->front = temp;
		queue->rear = temp;
	}
	else {
		queue->rear->next = temp;
		queue->rear = temp;
	}

}

void Queue_Dequeue(Queue* queue) {
	
	Queue_Node* temp = queue->front;
	if(queue->front != NULL) {
		if(queue->front->next != NULL) {
			queue->front = queue->front->next;
		} else {
			printf("queue front is null\n");
			queue->front = NULL;
			queue->rear = NULL;

		}
	}
	free(temp);


}

unsigned char* Queue_Front(Queue* queue) {
	if(queue->front == NULL) {
		printf("Front Queue is empty\n");
		return NULL;
	}
	return queue->front->data;
}

void Queue_Print(Queue* queue) {
	Queue_Node* temp = queue->front;
	while(temp != NULL) {
		printf("temp: %p\n", temp);
		printf("temp->next: %p\n", temp->next);
		char* format = Protocol_Format_Get(temp->data);
		printf("%s\n", format);
		Binary_Print(format, temp->data);
		free(format);
		temp = temp->next;
	}

}

void Queue_Destroy(Queue* queue) {
	if(queue != NULL) {
		free(queue);
	}
}

