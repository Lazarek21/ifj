/////////////////////////////////////////////////////////
//                                                     //
// Implementace překladače imperativního jazyka IFJ20. //
//                   ~ sa_stack.c ~                    //
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

//-------------------------------------------------------------------------
//                        STACK / LIST FUNCTIONS
//-------------------------------------------------------------------------
// Stack containing information about immersion and pointer to local
// symtable. It also has a poiter to next stack item, so it can be 
// used as list as well. It is used for keeping information about all
// variables and functions used by keeping different immersions in 
// different symtables. The first stack item is the most immersed one
// so if there's a need to find a variable, it finds the most recent use.

// initializes stack, if fails exits with error code 99 (internal error)
void sa_stack_init(T_SA_stack** SAStack)
{   
    *SAStack = malloc(sizeof(T_SA_stack));
    if(*SAStack == NULL)
        free_and_exit(INTERNAL_ERR);
    
    (*SAStack)->item_index = 0;
    (*SAStack)->label_index = 0;
    (*SAStack)->type = 0;
    (*SAStack)->next = NULL;
    (*SAStack)->local_symtab = NULL;
}

// pushes stack item, if it fails exits with error code 99 (internal error)
void sa_stack_push(T_SA_stack** SAStack, unsigned int temp_type){
    T_SA_stack* new_sa_stack;
    new_sa_stack = (T_SA_stack*)malloc(sizeof(T_SA_stack));

    if(new_sa_stack == NULL)
        free_and_exit(INTERNAL_ERR);
    new_sa_stack->item_index = ((*SAStack)->item_index+1);
    new_sa_stack->label_index = sa_data.label_counter;
    new_sa_stack->type = temp_type;
    new_sa_stack->next = (*SAStack);
    new_sa_stack->local_symtab = NULL;
    T_BTNode* temp = BT_insert(&(new_sa_stack->local_symtab),"_");
    temp->declared = true;
    temp->type = UNDERSCORE;
    *SAStack = new_sa_stack;
    
}

// free stack item at the top of the stack
void sa_stack_pop(T_SA_stack** SAStack)
{
    if(*SAStack != NULL){
        T_SA_stack* tmp = *SAStack;
        BT_dispose_tree(&(tmp->local_symtab));
        *SAStack = (*SAStack)->next;
        free(tmp);
    }
}

// disposes the whole stack
void sa_stack_empty(T_SA_stack** SAStack)
{ 
    while(*SAStack != NULL)
    {
        sa_stack_pop(SAStack);
    }
}
