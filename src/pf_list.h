typedef struct PF_Node {
	int g;
	int h;
	int f;
	
	int x;
	int y;

	struct PF_Node* prev;
	struct PF_Node* next;
	struct PF_Node* parent;

} PF_Node;

typedef struct PF_List {
	PF_Node* head;
	PF_Node* tail;
} PF_List;


PF_List* PF_Create();
void PF_Append(PF_List* list, PF_Node* node);
int PF_Length(PF_List* list);
PF_Node* PF_Get(PF_List* list, int x, int y);
PF_Node* PF_Clone(PF_Node* node); 
PF_Node* PF_Find_Min_FScore(PF_List* list);
void PF_Remove(PF_List* list, int x, int y);
void PF_Reverse(PF_List* list);
void PF_Print(PF_List* list, int print_index);
void PF_Destroy(PF_List* list);
void PF_Test_Find_Min_FScore();
void PF_Test_Get(); 
void PF_Test_Remove();


#define PF_LINKED_LIST_MAX 1024
