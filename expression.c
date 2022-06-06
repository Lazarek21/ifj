/////////////////////////////////////////////////////////
//                                                     //
// Implementace překladače imperativního jazyka IFJ20. //
//                  ~ expression.c ~                   //
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
typedef enum prec_vars { ND = -1, // NOT DEFINED
                  R = 0,  // REDUCE (>)
                  H = 1,  // HANDLE (<)
                  E = 2   // EQUAL (=)
} T_prec_vars;

const T_prec_vars prec_table[17][17] = {
//        |+ |- |* |/  |(  |)  |i |int |f |str |<  |>  |<= |>= |== |!= |$ |
/*  +  */ {R, R, H, H,  H,  R,  H,  H,  H,  H,  R,  R,  R,  R,  R,  R,  R},
/*  -  */ {R, R, H, H,  H,  R,  H,  H,  H,  H,  R,  R,  R,  R,  R,  R,  R},
/*  *  */ {R, R, R, R,  H,  R,  H,  H,  H,  H,  R,  R,  R,  R,  R,  R,  R},
/*  /  */ {R, R, R, R,  H,  R,  H,  H,  H,  H,  R,  R,  R,  R,  R,  R,  R},
/*  (  */ {H, H, H, H,  H,  E,  H,  H,  H,  H,  H,  H,  H,  H,  H,  H, ND},
/*  )  */ {R, R, R, R, ND,  R, ND, ND, ND, ND,  R,  R,  R,  R,  R,  R,  R},
/*  i  */ {R, R, R, R, ND,  R, ND, ND, ND, ND,  R,  R,  R,  R,  R,  R,  R},
/* int */ {R, R, R, R, ND,  R, ND, ND, ND, ND,  R,  R,  R,  R,  R,  R,  R},
/*float*/ {R, R, R, R, ND,  R, ND, ND, ND, ND,  R,  R,  R,  R,  R,  R,  R},
/*string*/{R, R, R, R, ND,  R, ND, ND, ND, ND,  R,  R,  R,  R,  R,  R,  R},
/* <   */ {H, H, H, H,  H,  R,  H,  H,  H,  H, ND, ND, ND, ND, ND, ND,  R},
/* >   */ {H, H, H, H,  H,  R,  H,  H,  H,  H, ND, ND, ND, ND, ND, ND,  R},
/* <=  */ {H, H, H, H,  H,  R,  H,  H,  H,  H, ND, ND, ND, ND, ND, ND,  R},
/* >=  */ {H, H, H, H,  H,  R,  H,  H,  H,  H, ND, ND, ND, ND, ND, ND,  R},
/* ==  */ {H, H, H, H,  H,  R,  H,  H,  H,  H, ND, ND, ND, ND, ND, ND,  R},
/* !=  */ {H, H, H, H,  H,  R,  H,  H,  H,  H, ND, ND, ND, ND, ND, ND,  R},
/*  $  */ {H, H, H, H,  H, ND,  H,  H,  H,  H,  H,  H,  H,  H,  H,  H, ND}
};


//-------------------------------------------------------------------------
//                           Expression function
//-------------------------------------------------------------------------
// check syntax of expressions
// based on bottom-up parsing
// cmp_disabled true - this expression mustn't contain relational operators
//              false - this expression has to contain relational operators, 
//              but only once
// dollar_transform - sets end of expression ({,;,\n)
// returns type of whole expression, if its not expression but function call 
// returns UNDEFINED

int expression(bool cmp_disabled, int dollar_transform)
{
    bool replaced = false;
    sa_data.cmp_used = cmp_disabled;
    SPA_init();
    int idx, m= 0;
    T_PA_data temp;

    while (!SPA_empty())
    { 
        
        if(TOKEN_TYPE == dollar_transform){
            TOKEN_TYPE = PA_ENDCHAR;
            replaced = true;
        } else if ( TOKEN_TYPE == END_LINE){
            replaced = false;
            TOKEN_TYPE =PA_ENDCHAR;
        } else if(TOKEN_TYPE > PA_ENDCHAR) { 
            free_and_exit(SYNTAX_ERR);
        } 
        if(TOKEN_TYPE == ID )
        {
            sa_data.previous_var_node = BT_search(sa_stack, sa_data.func_symtab, TOKEN_DATA);
            if(sa_data.previous_var_node == NULL)
            {  
                if ((sa_data.S).top > 1) //func musn't be part of expression 
                { 
                    free_and_exit(SYNTAX_ERR);
                }
                 
                FUNC_CURRENT = BT_search_and_insert(sa_data.func_symtab,NULL,TOKEN_DATA,FUNC_SYMTAB);
                if(get_token_cmp_type(L_PARENTH)){
                    get_token();
                    arg_list();
                } else free_and_exit(DEF_ERR);
                return UNDEFINED;
            }
        } 
        m = SPA_first_terminal();
        idx = (sa_data.S).arr[m].type;
        switch (prec_table[idx][TOKEN_TYPE])
        {
            case E:
                if(PA_STACK(0).terminal == false){
                    temp = PA_STACK(0);
                    PA_STACK(0) = PA_STACK(1);
                    SPA_pop();
                    PA_STACK(0) = temp;
                } else {
                    free_and_exit(SYNTAX_ERR);
                }
                get_token();
                break;
            case R:
                SPA_reduce_stack(&cmp_disabled);
                break;
            case ND:
                if((sa_data.S).top==2){
                    if(PA_STACK(1).type ==PA_ENDCHAR && PA_STACK(0).terminal == false && TOKEN_TYPE ==PA_ENDCHAR ){

                        code_insert("POPS GF@exp\n");
                    } else if (PA_STACK(1).type ==PA_ENDCHAR && PA_STACK(0).type == ID && TOKEN_TYPE == L_PARENTH){
                        FUNC_CURRENT = BT_search_and_insert(sa_data.func_symtab,NULL,TOKEN_DATA,FUNC_SYMTAB);
                        get_token();
                        arg_list();
                        SPA_dispose();
                        return UNDEFINED;
                    } else{
                        free_and_exit(SYNTAX_ERR);
                    }
                } else{
                    free_and_exit(SYNTAX_ERR);
                }
                temp = PA_STACK(0);
                SPA_dispose();
                break;
            case H:
                temp.handle = false;
                temp.terminal = true;
                temp.type = TOKEN_TYPE;
                
                if(strcmp(TOKEN_DATA,"0") == 0){
                    temp.is_zero = true;
                } else {
                    temp.is_zero = false;
                }
                if (TOKEN_TYPE == ID)
                {
                    temp.is_constant = false;
                } else {
                    temp.is_constant = true;
                }
                SPA_push(temp, TOKEN_DATA);
                (sa_data.S).arr[m+1].handle = true;              
                get_token();
                break;
            default:
                break;
        }
    }

    if(replaced){
        TOKEN_TYPE = dollar_transform;
    } else {
        TOKEN_TYPE = END_LINE;
    }
    return temp.type;
}
//-------------------------------------------------------------------------
//                           SPA_init function
//-------------------------------------------------------------------------
//  initilize stack for bottom-up analysis
//  first terminal is always PA_ENDCHAR type

void SPA_init ()
{
    sa_data.S.top = 0;
    T_PA_data temp = {true, false,PA_ENDCHAR, NULL, NULL, false, 0};
    sa_data.S.arr[0] =  temp;
    sa_data.S.top++;
}

void SPA_push (T_PA_data data, char* value)
{
    if (sa_data.S.top == MAXSTACK) {
	    free_and_exit(INTERNAL_ERR);
    } else {

	    sa_data.S.arr[sa_data.S.top] = data;

        sa_data.S.arr[sa_data.S.top].value = malloc(sizeof(char)*strlen(value));
        strcpy(sa_data.S.arr[sa_data.S.top].value, value);
        
        sa_data.S.top++;
    }
}

void SPA_pop ()
{
	if (sa_data.S.top==0)  {
		free_and_exit(INTERNAL_ERR);
	}
	else {
        free(PA_STACK(0).value);
		sa_data.S.top--;
	}
}


bool SPA_empty ()
{
    return(sa_data.S.top==0);
}

int SPA_first_terminal()
{
    for (int i = sa_data.S.top-1; i >= 0; i--)
    {
        if(sa_data.S.arr[i].terminal == true)
            return i;   
    } 
    return -1;
}



void SPA_reduce_stack (bool *cmp_disabled){
    if((PA_STACK(0).type >= ID && PA_STACK(0).type <= QM_STRING) && PA_STACK(0).terminal == true){
        
        if (PA_STACK(0).type == ID) {
            PA_STACK(0).type = sa_data.previous_var_node->type;
            PA_STACK(0).depth = sa_data.previous_var_node->depth;
            code_insert("PUSHS ");
            generate_type(ID,PA_STACK(0).value,PA_STACK(0).depth);
            code_insert("\n");
        } else {
            code_insert("PUSHS ");
            generate_type(PA_STACK(0).type,PA_STACK(0).value,PA_STACK(0).depth);
            code_insert("\n");
        }
        
        PA_STACK(0).terminal = false;
        PA_STACK(0).handle = false;
    } else if (PA_STACK(0).terminal == false){
        if(sa_data.S.top >3){    
            if(PA_STACK(0).type != PA_STACK(2).type)
                free_and_exit(TYPE_ERR);
            if((PA_STACK(1).type >= PLUS )&& (PA_STACK(1).type <= EQUAL) && PA_STACK(2).terminal == false)
            {
                code_gen_operation();
                T_PA_data temp = PA_STACK(0);
                SPA_pop();
                SPA_pop();
                SPA_pop();
                temp.handle = false;
                SPA_push(temp,"$"); 
            } else free_and_exit(SYNTAX_ERR);
        } else free_and_exit(SYNTAX_ERR);
    } else  free_and_exit(SYNTAX_ERR);
}

void SPA_dispose()
{
    while(sa_data.S.top > 0){
        SPA_pop();
    }
}
