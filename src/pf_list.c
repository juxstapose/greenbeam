#include <stdlib.h>
#include <stdio.h>
#include "pf_list.h"

PF_List* PF_Create() {
	PF_List* result = (PF_List*)malloc(sizeof(PF_Node));
	
	result->head = (PF_Node*)malloc(sizeof(PF_Node));
	result->head->x = 0;
	result->head->y = 0;
	//printf("head: [%p]\n", result->head);
	result->tail = (PF_Node*)malloc(sizeof(PF_Node));
	result->tail->x = 0;
	result->tail->y = 0;
	//printf("tail: [%p]\n", result->tail);
		
	result->head->prev = NULL;
	result->head->next = result->tail;
	
	result->tail->prev = result->head;
	result->tail->next = NULL;
	
	return result;
}

	
void PF_Append(PF_List* list, PF_Node* node) { 
	//printf("start append\n");
	int length = PF_Length(list);
	///printf("length: %i\n", length);	
	if(length == 0) {
		//printf("head: [%p] x:%i y:%i\n", list->head, list->head->x, list->head->y);
		//printf("tail: [%p] x:%i y:%i\n", list->tail, list->tail->x, list->tail->y);
		//printf("node: [%p] x:%i y:%i\n", node, node->x, node->y);
		list->head->next = node;
		list->tail->prev = node;
		node->next = list->tail;
		node->prev = list->head;
		//printf("\n");
		//printf("head: [%p] x:%i y:%i\n", list->head, list->head->x, list->head->y);
		//printf("tail: [%p] x:%i y:%i\n", list->tail, list->tail->x, list->tail->y);
		//printf("node: [%p] x:%i y:%i\n", node, node->x, node->y);
		//printf("\n");
	} else if(length < 0) {
		fprintf(stderr, "negative length invalid list");
	} else if(length > PF_LINKED_LIST_MAX) {
		fprintf(stderr, "list exceeded defined maximum of %i", PF_LINKED_LIST_MAX);
	}
	else {
		//printf("head: [%p] x:%i y:%i\n", list->head, list->head->x, list->head->y);
		//printf("tail: [%p] x:%i y:%i\n", list->tail, list->tail->x, list->tail->y);
		//printf("node: [%p] x:%i y:%i\n", node, node->x, node->y);
		PF_Node* temp = list->tail->prev; 	
		//printf("\n");
		list->tail->prev = node;
		node->next = list->tail;
		node->prev = temp;
		temp->next = node; 
		//printf("head: [%p] x:%i y:%i\n", list->head, list->head->x, list->head->y);
		//printf("tail: [%p] x:%i y:%i\n", list->tail, list->tail->x, list->tail->y);
		//printf("node: [%p] x:%i y:%i\n", node, node->x, node->y);
		//printf("\n");
	}
}

int PF_Length(PF_List* list) {
	//printf("start length\n");
	int result = 0;
	PF_Node *node = list->head;
	//printf("start loop\n");
	while(node != NULL) {
		//printf("inside loop\n");
		//printf("node:%p\n", node);
		//printf("node: [%p] x:%i y:%i\n", node, node->x, node->y);
		node = node->next;
		result++;
	}
	//printf("end length\n");
	return result - 2;
}

void PF_Remove(PF_List* list, int x, int y) {
	PF_Node* node = list->head;
	PF_Node* node_to_remove = NULL;
	while(node != NULL) {
		if(node != list->head && node != list->tail) {
			//printf("node: [%p] x:%i y:%i\n", node, node->x, node->y);
			if(node->x == x && node->y == y) {
				node_to_remove = node;
				node->prev->next = node->next;
				node->next->prev = node->prev;
				//printf("removing [%p] x:%i y:%i\n", node, node->x, node->y);	
				//printf("node to remove: [%p]\n", node_to_remove);
				//free(node);
				//node = NULL;
			}
		}
		node = node->next;
	}
	if(node_to_remove != NULL) {
		//printf("removing node [%p]\n", node_to_remove);
		free(node_to_remove);
		node_to_remove = NULL;
	}
	//printf("after node free\n");
}

PF_Node* PF_Get(PF_List* list, int x, int y) {
	PF_Node* node = list->head;
	while(node != NULL) {
		if(node != list->head && node != list->tail) {
			//printf("node: [%p] x:%i y:%i\n", node, node->x, node->y);
			if(node->x == x && node->y == y) {
				return node;
			}
		}
		node = node->next;
	}
	return NULL;
}

PF_Node* PF_Clone(PF_Node* node) {
	if(node != NULL) {	
		//printf("start clone\n");
		PF_Node* result = (PF_Node*)malloc(sizeof(PF_Node));
		result->x = node->x;
		result->y = node->y;	
		//printf("before parent assignment\n");
		result->parent = node->parent;
		//printf("after parent assignment\n");
		result->prev = node->prev;
		result->next = node->next;
		result->g = node->g;
		result->h = node->h;
		result->f = node->f;
		//printf("end clone\n");
		return result;
	} 
	return NULL;
}

PF_Node* PF_Find_Min_FScore(PF_List* list) {
	int f = 999999999;
	int x = 0;
	int y = 0;
	PF_Node* node = list->head;
	while(node != NULL) {
		if(node != list->head && node != list->tail) {
			if(node->f < f) {
				f = node->f;
				x = node->x;
				y = node->y;
			}
		}
		node = node->next;
	}
	PF_Node* result = PF_Get(list, x, y);
	return result;
}

void PF_Reverse(PF_List* list) {
	PF_Node* node = list->head;
	PF_Node* temp = NULL;
	while(node != NULL) {
		temp = node->prev;
		node->prev = node->next;
		node->next = temp;
		node = node->prev;
		if(temp != NULL) { 
			list->head = temp->prev;
		}
	}
}

void PF_Print(PF_List* list, int print_index) {
	int index = 0;
	PF_Node* node = list->head;
	int length = PF_Length(list) + 2;
	while(node != NULL) {
		if(print_index < 0) {
			//printf("index: %i\n", index);
			if(index == 0) {
				printf("[%i,%i]:%i ", node->x, node->y, node->f);
			} else if(index == length-1) { 
				printf(" [%i,%i]:%i\n", node->x, node->y, node->f);
			} else {
				printf(" [%i,%i]:%i ", node->x, node->y, node->f);
			}
		} else if (print_index >= 0) {
			if(index == print_index) {
				printf("[%i,%i]:%i\n", node->x, node->y, node->f);
				break;
			}
		}
		node = node->next;
		index++;
	}
}

void PF_Destroy(PF_List* list) {
	PF_Node* node = list->head;
	//printf("length:%i\n", PF_Length);
	while(node != NULL) {
		PF_Node* temp = node;
		//printf("[%p] x:%i y:%i\n", node, node->x, node->y);
		node = node->next;
		free(temp);
		temp = NULL;
	}
	list->head = NULL;
	list->tail = NULL;
	free(list);
}

void PF_Test_Reverse() { 

	printf("start test reverse\n");	
	PF_List* list = PF_Create();
	
	PF_Node* node_one = (PF_Node*)malloc(sizeof(PF_Node));
	node_one->x = 25;
	node_one->y = 10;
	node_one->f = 54;
	PF_Append(list, node_one);
	
	PF_Node* node_two = (PF_Node*)malloc(sizeof(PF_Node));
	node_two->x = 49;
	node_two->y = 56;
	node_two->f = 24;
	PF_Append(list, node_two);

	PF_Node* node_three = (PF_Node*)malloc(sizeof(PF_Node));
	node_three->x = 4;
	node_three->y = 7;
	node_three->f = 40;
	PF_Append(list, node_three);
	
	PF_Node* node_four = (PF_Node*)malloc(sizeof(PF_Node));
	node_four->x = 42;
	node_four->y = 1;
	node_four->f = 10;
	PF_Append(list, node_four);

	PF_Print(list, -1);
	PF_Reverse(list);
	PF_Print(list, -1);
	
	PF_Destroy(list);
	
	list = PF_Create();

	node_one = (PF_Node*)malloc(sizeof(PF_Node));
	node_one->x = 25;
	node_one->y = 10;
	node_one->f = 54;
	PF_Append(list, node_one);
	
	node_two = (PF_Node*)malloc(sizeof(PF_Node));
	node_two->x = 49;
	node_two->y = 56;
	node_two->f = 24;
	PF_Append(list, node_two);

	node_three = (PF_Node*)malloc(sizeof(PF_Node));
	node_three->x = 4;
	node_three->y = 7;
	node_three->f = 40;
	PF_Append(list, node_three);
	
	node_four = (PF_Node*)malloc(sizeof(PF_Node));
	node_four->x = 42;
	node_four->y = 1;
	node_four->f = 10;
	PF_Append(list, node_four);
	
	PF_Node* node_five = (PF_Node*)malloc(sizeof(PF_Node));
	node_five->x = 2;
	node_five->y = 11;
	node_five->f = 99;
	PF_Append(list, node_five);
	
	PF_Print(list, -1);
	PF_Reverse(list);
	PF_Print(list, -1);
	
	PF_Destroy(list);
	
	list = PF_Create();

	node_one = (PF_Node*)malloc(sizeof(PF_Node));
	node_one->x = 25;
	node_one->y = 10;
	node_one->f = 54;
	PF_Append(list, node_one);
	
	PF_Print(list, -1);
	PF_Reverse(list);
	PF_Print(list, -1);
	
	PF_Destroy(list);

	printf("end test reverse\n");	

}


void PF_Test_Find_Min_FScore() {
	printf("start find min test\n");
	PF_List* list = PF_Create();
	
	PF_Node* node_one = (PF_Node*)malloc(sizeof(PF_Node));
	node_one->x = 25;
	node_one->y = 10;
	node_one->f = 54;
	PF_Append(list, node_one);
	
	PF_Node* node_two = (PF_Node*)malloc(sizeof(PF_Node));
	node_two->x = 49;
	node_two->y = 56;
	node_two->f = 24;
	PF_Append(list, node_two);

	PF_Node* node_three = (PF_Node*)malloc(sizeof(PF_Node));
	node_three->x = 4;
	node_three->y = 7;
	node_three->f = 40;
	PF_Append(list, node_three);
	
	PF_Print(list, -1);

	PF_Node* result = PF_Find_Min_FScore(list);
	if(result->f == node_two->f) {
		printf("test 1 passed\n");
	} else {
		printf("test 1 failed\n");
	}
	printf("end find min test\n");
}

void PF_Test_Get() { 
	printf("start get test\n");
	PF_List* list = PF_Create();
	
	PF_Node* node_one = (PF_Node*)malloc(sizeof(PF_Node));
	node_one->x = 20;
	node_one->y = 10;
	PF_Append(list, node_one);
	
	PF_Node* node_two = (PF_Node*)malloc(sizeof(PF_Node));
	node_two->x = 4;
	node_two->y = 16;
	
	PF_Node* found_one = PF_Get(list, node_one->x, node_one->y);
	int in_list = (found_one == NULL) ? 0 : 1;
	if(in_list) { 
		printf("found node one x:%i y:%i\n", found_one->x, found_one->y);
		printf("test 1 passed\n");
	} else { 
		printf("not found x:%i y:%i\n", node_one->x, node_one->y);
		printf("test 1 failed\n");
	}

	PF_Node* found_two = PF_Get(list, node_two->x, node_two->y);
	in_list = (found_two == NULL) ? 0 : 1;
	if(in_list) { 
		printf("found node two x:%i y:%i\n", found_two->x, found_two->y);
		printf("test 2 failed\n");
	} else { 
		printf("not found x:%i y:%i\n", node_two->x, node_two->y);
		printf("test 2 passed\n");
	}
	
	PF_Destroy(list);
	printf("end get test\n");
}


void PF_Test_Remove() {
	printf("start remove test\n");
	
	int length = 0;

	PF_List* list = PF_Create();
	PF_Node* node_one = (PF_Node*)malloc(sizeof(PF_Node));
	node_one->x = 10;
	node_one->y = 5;
	PF_Append(list, node_one);
	
	PF_Node* node_two = (PF_Node*)malloc(sizeof(PF_Node));
	node_two->x = 15;
	node_two->y = 7;
	PF_Append(list, node_two);

	PF_Node* node_three = (PF_Node*)malloc(sizeof(PF_Node));
	node_three->x = 20;
	node_three->y = 10;
	PF_Append(list, node_three);

	PF_Node* node_four = (PF_Node*)malloc(sizeof(PF_Node));
	node_four->x = 40;
	node_four->y = 30;
	PF_Append(list, node_four);
	
	printf("full list\n");
	PF_Print(list, -1);		
	
	length = PF_Length(list);
	
	printf("length: %i remove x:%i and y:%i\n", length, node_three->x, node_three->y);
	PF_Remove(list, node_three->x, node_three->y);		
	
	PF_Print(list, -1);
	
	length = PF_Length(list);
	if(length == 3) {
		printf("test 1 passed\n");
	}
		
	PF_Remove(list, node_one->x, node_one->y);
	PF_Print(list, -1);
	length = PF_Length(list);
	if(length == 2) {
		printf("test 2 passed\n");
	}
	
	PF_Remove(list, node_four->x, node_four->y);
	PF_Print(list, -1);
	length = PF_Length(list);
	if(length == 1) {
		printf("test 3 passed\n");
	}
	
	PF_Remove(list, node_two->x, node_two->y);
	PF_Print(list, -1);
	length = PF_Length(list);
	if(length == 0) {
		printf("test 4 passed\n");
	}
	
	PF_Node* node_five = (PF_Node*)malloc(sizeof(PF_Node));
	node_five->x = 40;
	node_five->y = 30;
	PF_Append(list, node_five);
	PF_Print(list, -1);
	length = PF_Length(list);
	if(length == 1) {
		printf("test 5 passed\n");
	}

	PF_Destroy(list);
	printf("end remove test\n");
}
