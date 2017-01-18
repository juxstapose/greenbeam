
#define G_SCORE_DIAG 14

#define G_SCORE_HV 10

#define H_SCORE_FACTOR 10

int PF_Is_Corner(PF_List* wall_list, PF_Node* selected_node, int x, int y);
void PF_Pathfind(PF_List* open_list, PF_List* closed_list, PF_List* wall_list, PF_Node* selected_node, PF_Node* end_node, int x, int y);
void PF_Find_Adjacent_Squares(PF_List* open_list, PF_List* closed_list, PF_List* wall_list, PF_Node* selected_node, PF_Node* end_node);	
int PF_Is_Diag(PF_Node * node, int x, int y);
int PF_Calc_G(PF_Node* node); 
int PF_Calc_H(PF_Node* start, PF_Node* end);
PF_List* PF_Find_Path(PF_Node* start, PF_Node* target); 
PF_List* PF_Get_Vert_Wall();
void PF_TestPathfind(PF_List* wall_list, PF_Node* start_node, PF_Node* end_node);
