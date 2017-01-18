#include <stdlib.h>
#include <stdio.h>
#include "pf_list.h"
#include "pathfinding.h"

int PF_Is_Corner(PF_List* wall_list, PF_Node* selected_node, int x, int y) {
	int is_wall_lr_right = 0;
	int is_wall_lr_left = 0;
	int is_wall_rl_right = 0;
	int is_wall_rl_left = 0;
	
	//int is_wall_rl_right = 0;
	//int is_wall_rl_left = 0;
	
	PF_Node* adj_wall_left_lr_node = PF_Get(wall_list, x-1, y);
	PF_Node* adj_wall_right_lr_node = PF_Get(wall_list, selected_node->x+1, selected_node->y);		
	
	PF_Node* adj_wall_left_rl_node = PF_Get(wall_list, selected_node->x-1, selected_node->y);
	PF_Node* adj_wall_right_rl_node = PF_Get(wall_list, x+1, y);
	
	int is_diag = PF_Is_Diag(selected_node, x, y);	
	
	//left to right
	is_wall_lr_left = (adj_wall_left_lr_node == NULL) ? 0 : 1;
	is_wall_lr_right = (adj_wall_right_lr_node == NULL) ? 0 : 1;
	printf("(%i,%i) lr wall right: %i is_diag: %i\n", x, y, is_wall_lr_right, is_diag);
	printf("(%i,%i) lr wall left: %i is_diag: %i\n", x, y, is_wall_lr_left, is_diag);
	
	//right to left	
	is_wall_rl_left = (adj_wall_left_rl_node == NULL) ? 0 : 1;
	is_wall_rl_right = (adj_wall_right_rl_node == NULL) ? 0 : 1;
	printf("(%i,%i) rl wall right: %i is_diag: %i\n", x, y, is_wall_rl_right, is_diag);
	printf("(%i,%i) rl wall left: %i is_diag: %i\n", x, y, is_wall_rl_left, is_diag);
	
	printf("selected node x:%i - x:%i = %i\n", selected_node->x, x, selected_node->x - x);
	int left_to_right = (selected_node->x - x < 0) ? 1 : 0;
	int right_to_left = (selected_node->x - x >= 0) ? 1 : 0;

	if(left_to_right == 1) {
		if( (is_wall_lr_right ==  1 && is_diag == 1) || 
		    (is_wall_lr_left ==  1 && is_diag == 1) ) {
			return 1;
		}
	}
	else if(right_to_left == 1) {
		if( (is_wall_rl_right ==  1 && is_diag == 1) || 
		    (is_wall_rl_left ==  1 && is_diag == 1) ) {
			return 1;
		}

	}

	return 0;
}	


void PF_Pathfind(PF_List* open_list, PF_List* closed_list, PF_List* wall_list, PF_Node* selected_node, PF_Node* end_node, int x, int y) {
	PF_Node* closed_list_node = PF_Get(closed_list, x, y);
	PF_Node* open_list_node = PF_Get(open_list, x, y);
	PF_Node* wall_node = PF_Get(wall_list, x, y);
	
	
	int is_wall_node = (wall_node == NULL) ? 0 : 1;
	int in_closed_list = (closed_list_node == NULL) ? 0 : 1;
	int in_open_list = (open_list_node == NULL) ? 0 : 1;
	
	//have we seen this node yet
	if(in_closed_list == 0) {
		if(in_open_list == 0) {
			//not on the open list so create it and put it in
			//printf("is_wall_node: %i (%i,%i)\n", is_wall_node,x, y);
			//if next to a wall node and diagnol is open then go through vertical space instead
			
			if(is_wall_node == 0) {
				//if( (is_wall_right ==  1 && is_diag == 1) || 
				//    (is_wall_left ==  1 && is_diag == 1) ) {
				if(PF_Is_Corner(wall_list, selected_node, x, y) == 1) {
					printf("ignoring parent:(%i,%i) node:(%i,%i)\n", selected_node->x, selected_node->y, x, y);
				
				} else {
					PF_Node* node = (PF_Node*)malloc(sizeof(PF_Node));
					node->x = x;
					node->y = y;	
					node->parent = selected_node;
					//printf("parent: %p (%i,%i)\n", node->parent, node->parent->x, node->parent->y);
					node->g = PF_Calc_G(node);
					node->h = PF_Calc_H(node, end_node);
					node->f = node->g + node->h;
					//printf("g:%i h:%i f:%i\n", node->g, node->h, node->f);
					//printf("is_wall_node: %i (%i,%i)\n", is_wall_node,x, y);
					PF_Append(open_list, node);
				} 
			}	
		} else if(in_open_list == 1) {
			//check if g score of the selected square + current square
			//we make the selected square the parent and test the g score by comparing it with the original gscore
			//if test passes then we keep it as parent
			//save old parent
			PF_Node* old_parent = open_list_node->parent;
			//printf("in open list: old parent: %p\n", old_parent);		
			//calculate g score with old parent 
			int old_g = PF_Calc_G(open_list_node);
			//make selected node the new parent
			open_list_node->parent = selected_node;
			//calculate new g score
			int new_g = PF_Calc_G(open_list_node);
			//compare g scores
			if(new_g < old_g) {
				//new g is a better path re calculate f and h scores
				open_list_node->g = PF_Calc_G(open_list_node);
				open_list_node->h = PF_Calc_H(open_list_node, end_node);
				open_list_node->f = open_list_node->g + open_list_node->h;
			} else {
				//put back to original parent
				open_list_node->parent = old_parent;
			}	
		}
	} else if(is_wall_node == 1) {
		//printf("is_wall_node: %i (%i,%i)\n", is_wall_node,x, y);
	}
}



void PF_Find_Adjacent_Squares(PF_List* open_list, PF_List* closed_list, PF_List* wall_list, PF_Node* selected_node, PF_Node* end_node) {	
	//make nodes that surround square if they do not already exist
	int x = selected_node->x;
	int y = selected_node->y;
	printf("selected node: %p %i,%i f:%i\n", selected_node, selected_node->x, selected_node->y, selected_node->f);
	PF_Pathfind(open_list, closed_list, wall_list, selected_node, end_node, x-1, y-1);
	PF_Pathfind(open_list, closed_list, wall_list, selected_node, end_node, x-1, y);
	PF_Pathfind(open_list, closed_list, wall_list, selected_node, end_node, x-1, y+1);
	PF_Pathfind(open_list, closed_list, wall_list, selected_node, end_node, x, y-1);
	PF_Pathfind(open_list, closed_list, wall_list, selected_node, end_node, x, y+1);
	PF_Pathfind(open_list, closed_list, wall_list, selected_node, end_node, x+1, y-1);
	PF_Pathfind(open_list, closed_list, wall_list, selected_node, end_node, x+1, y);
	PF_Pathfind(open_list, closed_list, wall_list, selected_node, end_node, x+1, y+1);
}

int PF_Is_Diag(PF_Node* test, int x, int y) {
	if(test->x == x-1 && test->y == y-1) {
		return 1;
	}
	if(test->x == x-1 && test->y == y+1) {
		return 1;
	}
	if(test->x == x+1 && test->y == y-1) {
		return 1;
	}
	if(test->x == x+1 && test->y == y+1) {
		return 1;
	}
	return 0;
}

int PF_Calc_G(PF_Node* node) { 
	int result = 0;	
	int is_diag = PF_Is_Diag(node, node->parent->x, node->parent->y);
	//int is_adj = PF_Is_Adj(node);
	if(is_diag) {
		result = node->parent->g + 14;
		return result;
	}
	result = node->parent->g + 10;
	return result;
}

int PF_Calc_H(PF_Node* start, PF_Node* end) {
	int distance_x = abs(start->x - end->x);
	int distance_y = abs(start->y - end->y);
	return (distance_x + distance_y) * 10;
}

PF_List* PF_Find_Path(PF_Node* start, PF_Node* target) { 
	PF_List* result = PF_Create();
	PF_Node* node = target;
	//printf("before while %p %p\n", start, node);
	//printf("start (%i,%i)\n", start->x, start->y);
	//printf("target (%i,%i)\n", node->x, node->y);
	
	while( !(node->x == start->x && node->y == start->y) ) {
		printf("[%p] %i,%i\n", node, node->x, node->y);
		PF_Append(result, node);
		//printf("node parent: %p\n", node->parent);
		//printf("node parent x: %i\n", node->parent->x);
		//printf("node parent y: %i\n", node->parent->y);
		//printf("node parent: %p (%i,%i)\n", node->parent, node->parent->x, node->parent->y);
		node = node->parent;
		if(node == NULL) {
			printf("found null breaking\n");
			break;
		}
	}
	return result;

}

PF_List* PF_Get_Vert_Wall() {
	PF_List* wall_list = PF_Create();
	
	PF_Node* node_one = (PF_Node*)malloc(sizeof(PF_Node));
	node_one->x = 3;
	node_one->y = 1;
	PF_Append(wall_list, node_one);

	PF_Node* node_two = (PF_Node*)malloc(sizeof(PF_Node));
	node_two->x = 3;
	node_two->y = 2;
	PF_Append(wall_list, node_two);

	PF_Node* node_three = (PF_Node*)malloc(sizeof(PF_Node));
	node_three->x = 3;
	node_three->y = 3;
	PF_Append(wall_list, node_three);

	return wall_list;
}

PF_List* PF_Get_Horiz_Wall() {
	PF_List* wall_list = PF_Create();
	
	PF_Node* node_one = (PF_Node*)malloc(sizeof(PF_Node));
	node_one->x = 2;
	node_one->y = 2;
	PF_Append(wall_list, node_one);

	PF_Node* node_two = (PF_Node*)malloc(sizeof(PF_Node));
	node_two->x = 3;
	node_two->y = 2;
	PF_Append(wall_list, node_two);

	PF_Node* node_three = (PF_Node*)malloc(sizeof(PF_Node));
	node_three->x = 4;
	node_three->y = 2;
	PF_Append(wall_list, node_three);

	return wall_list;
}

void PF_TestPathfind(PF_List* wall_list, PF_Node* start_node, PF_Node* end_node) {

	PF_List* open_list = PF_Create();
	PF_List* closed_list = PF_Create();

	//add starting square to open list 
	PF_Append(open_list, start_node);	
	
	PF_Node* min_node = NULL;
	PF_Node* selected_node = NULL;
	PF_Node* result = NULL;
	int i = 0;

	while(1) {	
		
		//printf("find min node\n");
		PF_Print(open_list, -1);
		min_node = PF_Find_Min_FScore(open_list);	
		//printf("end find min node\n");
		//printf("min_node: [%p]\n", min_node);
		selected_node = PF_Clone(min_node);
		//remove from open list and place it on closed list
		//printf("remove\n");
		PF_Remove(open_list, min_node->x, min_node->y);
		//printf("append\n");
		PF_Append(closed_list, selected_node);	
		

		printf("before open list\n");
		PF_Print(open_list, -1);
		printf("before closed list\n");
		PF_Print(closed_list, -1);
		//printf("find adjacent quares\n");
		PF_Find_Adjacent_Squares(open_list, closed_list, wall_list, selected_node, end_node);	
		printf("before open list\n");
		PF_Print(open_list, -1);
		printf("after closed list\n");
		PF_Print(closed_list, -1);
		
		//printf("find target square\n");	
		result = PF_Get(closed_list, end_node->x, end_node->y);
		if(result != NULL) {
			printf("found target square\n");
			break;
		}
		
		/**
		if(i == 2) {
			break;
		}
		i++;
		**/	
	}
	
	printf("final result\n");
	PF_List* path = PF_Find_Path(start_node, result);
	PF_Print(path, -1);
	PF_Reverse(path);
	PF_Print(path, -1);

}
