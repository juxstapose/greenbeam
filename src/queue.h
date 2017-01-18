#ifndef _QUEUE_H_
#define _QUEUE_H_

typedef struct Queue_Node {
	unsigned char* data;
	struct Queue_Node* next;
} Queue_Node;

typedef struct Queue {
	Queue_Node* front;
	Queue_Node* rear;
} Queue;

Queue* Queue_Create();
void Queue_Enqueue(Queue* queue, unsigned char* data);
void Queue_Dequeue(Queue* queue);
unsigned char* Queue_Front(Queue* queue);
void Queue_Print(Queue* queue);
void Queue_Destroy(Queue* queue);

#endif
