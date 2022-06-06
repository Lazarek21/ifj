/////////////////////////////////////////////////////////
//                                                     //
// Implementace překladače imperativního jazyka IFJ20. //
//                    ~ symtab.c ~                     //
//                                                     //
//              Hynek Šabacký (xsabac02),              //
//             Mikuláš Brázda (xbrazd21),              //
//              David Lorenc (xloren16)                //
//                                                     //
//               Last edit: 5. 12. 2020                //
///////////////////////////////////////////////////////// 

#include "main.h"
extern T_SA_stack* sa_stack;
extern T_SA_data sa_data;

void S_init (tStackP *S)
{
	S->top = 0;
}
void S_push (tStackP *S, T_BTNode* ptr)
{
  if (S->top==MAXSTACK) {
	  free_and_exit(INTERNAL_ERR);
  } else {
		S->top++;
		S->a[S->top]=ptr;
	}
}

T_BTNode* S_top_pop (tStackP *S)
{
	if (S->top==0)  {
		return(NULL);
	}
	else {
		return (S->a[S->top--]);
	}
}

bool S_empty (tStackP *S)
{
  return(S->top==0);
}


//-------------------------------------------------------------------------
//                           BT_search function
//-------------------------------------------------------------------------
// goes through whole sa_stack from start to end
// returns first match of Key (simulates bahavior of variables shadowing)

T_BTNode* BT_search(T_SA_stack* start, T_SA_stack* end, const char* Key){
	T_SA_stack* TempStack = start; 
	T_BTNode* PrevNode = NULL;
	bool found = false;
	while(TempStack != end && !found){
		PrevNode = BT_local_search(TempStack->local_symtab, Key);
		if(PrevNode == NULL)
		{
			TempStack = TempStack->next;
		}
		else {
			found = true;
		}
	}
	return PrevNode;
}

//-------------------------------------------------------------------------
//                           BT_local_search function
//-------------------------------------------------------------------------
// Binary search algorithm - Iterative implementation
// if successed returns pointer to found variable else returns NULL

T_BTNode* BT_local_search(T_BTNode* RootPtr, const char* Key)
{
	T_BTNode* TempNode = RootPtr;
	int left_right;
	while (TempNode != NULL)
	{
		left_right = strcmp(TempNode->Key,Key);
		if (left_right < 0)
		{
			TempNode = TempNode->RPtr;
		} else if(left_right > 0){
			TempNode = TempNode->LPtr;
		} else {
			return TempNode;
		}
	}
	return NULL;
}
//-------------------------------------------------------------------------
//                           BT_insert function
//-------------------------------------------------------------------------
// returns pointer to newly inserted variable
// always used with knowledge that variable with name Key doesnt exist
T_BTNode* BT_insert(T_BTNode** RootPtr, const char* Key) 
{

	int left_right;
	T_BTNode* TempNode = *RootPtr, *TempRoot;
	
	while (TempNode != NULL)
	{
		TempRoot = TempNode;
		left_right = strcmp(TempNode->Key,Key);
		if (left_right < 0)
		{
			TempNode = TempNode->RPtr;
		} else {
			TempNode = TempNode->LPtr;
		} 
	}

	TempNode = malloc(sizeof(T_BTNode));
	if(TempNode == NULL)
	{
		free_and_exit(INTERNAL_ERR);
	}
	
	TempNode->Key = malloc(strlen(Key)*sizeof(char));
	if (TempNode->Key == NULL)
	{
		free_and_exit(INTERNAL_ERR);
	}
	if(sa_data.retval_counter < 0){
		strcpy(TempNode->Key,Key);
	} else {
		sprintf(TempNode->Key, "%s%d", Key, sa_data.retval_counter);
	}
	TempNode->depth = sa_stack->item_index;
	TempNode->LPtr = NULL;
	TempNode->RPtr = NULL;
	TempNode->rets = NULL;
	TempNode->args = NULL;
	TempNode->declared = false;
	TempNode->used = false;
	if (*RootPtr != NULL)
	{
		if(left_right < 0)
		{
			TempRoot->RPtr = TempNode;
		} else {
			TempRoot->LPtr = TempNode;
		}
	} else {
		*RootPtr = TempNode;
	}
	return TempNode;
}

//-------------------------------------------------------------------------
//                           BT_search_and_insert function
//-------------------------------------------------------------------------
// if it doesnt find variable with name Key automatically inserts
// returns pointer to newly inserted variable or to found variable

T_BTNode* BT_search_and_insert(T_SA_stack* start, T_SA_stack* end, const char* Key, T_BTNode** dest)
{
	T_BTNode* temp = BT_search(start,end,Key);
	if (temp == NULL)
		temp = BT_insert(dest,Key);

	return temp;
}

void BT_dispose_tree (T_BTNode** RootPtr)	
{
	tStackP S;
	S_init(&S);
	T_BTNode* TempNode = *RootPtr;
	T_type_list* temp;
	if (*RootPtr != NULL)
	{
		S_push(&S,*RootPtr);
		while(!S_empty(&S))
		{
			TempNode = S_top_pop(&S);
			if (TempNode->RPtr != NULL)
			{
				S_push(&S,TempNode->RPtr);
			}  
			if (TempNode->LPtr != NULL){
				S_push(&S,TempNode->LPtr);
			}
			while (TempNode->args != NULL)
			{	
				temp = TempNode->args;
				TempNode->args = TempNode->args->next;
				free(temp);
			}
			while (TempNode->rets != NULL)
			{	
				temp = TempNode->rets;
				TempNode->rets = TempNode->rets->next;
				free(temp);
			}
			free(TempNode->Key);
			free(TempNode);
		}
	}
	*RootPtr = NULL;
}


void Leftmost_Inorder(T_BTNode* ptr, tStackP *Stack)		{


	while(ptr != NULL)
	{
		S_push(Stack,ptr);
		ptr = ptr->LPtr;
	}

}

//-------------------------------------------------------------------------
//                           BTInorder function
//-------------------------------------------------------------------------
// Goes through whole binary tree and perform operation inorder 

void BTInorder (T_BTNode* RootPtr, void (*operation)(T_BTNode*))	{

	tStackP S;
	S_init(&S);
	T_BTNode* TempNode = RootPtr;
	Leftmost_Inorder(RootPtr,&S);
	while(!S_empty(&S) && TempNode != NULL)
	{
		TempNode = S_top_pop(&S);
		operation(TempNode);
		
		if(TempNode->RPtr != NULL)
		{
			Leftmost_Inorder(TempNode->RPtr,&S);
		}
	}
}


