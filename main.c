/////////////////////////////////////////////////////////
//                                                     //
// Implementace překladače imperativního jazyka IFJ20. //
//                    ~ main.c ~                       //
//                                                     //
//              Hynek Šabacký (xsabac02),              //
//             Mikuláš Brázda (xbrazd21),              //
//              David Lorenc (xloren16)                //
//                                                     //
//               Last edit: 5. 12. 2020                //
///////////////////////////////////////////////////////// 


#include "main.h"

T_SA_data sa_data;
T_SA_stack* sa_stack;

int main(int argc, char **argv)
{
    InitSa_data();
    sa_stack_init(&sa_stack);
    sa_data.func_symtab = sa_stack;
    Init_func_symtab(FUNC_SYMTAB);
    get_token();
    prologue();
    return 0;
}

void InitSa_data()
{
    sa_data.main_found = false;
    sa_data.inside_return = false;
    sa_data.return_found = false;

    sa_data.token = (T_token*) malloc(sizeof(T_token));
    if (sa_data.token == NULL)
        free_and_exit(INTERNAL_ERR);
    TOKEN_DATA = NULL;
    TOKEN_TYPE = UNDEFINED;
    FUNC_CURRENT = NULL;
    VAR_CURRENT = NULL;
    sa_data.func_symtab = NULL;
    sa_data.previous_var_node = NULL;
    sa_data.left_side_types = NULL;
    sa_data.right_side_types = NULL;
    sa_data.inside_func = NULL;
    sa_data.cmp_used = false;

    sa_data.main_code.alocated = CODE_SIZE;
    sa_data.main_code.used = 0;
    sa_data.main_code.output = NULL;
    sa_data.func_code.output = NULL;
    sa_data.main_code.output = (char*) malloc(sizeof(char)*CODE_SIZE);
    if(sa_data.main_code.output == NULL) 
        free_and_exit(INTERNAL_ERR);
    
    sa_data.func_code.alocated = CODE_SIZE;
    sa_data.func_code.used = 0;
    sa_data.func_code.output = (char*) malloc(sizeof(char)*CODE_SIZE);
    if(sa_data.func_code.output == NULL) 
        free_and_exit(INTERNAL_ERR);

    sa_data.retval_counter = -1;
    sa_data.label_counter = 0;
    sa_data.left_side_ids = NULL;
}

//-------------------------------------------------------------------------
//                          Init_func_symtab function
//-------------------------------------------------------------------------
// Initilize function symtable with builtin functions
// Builtin functions are ordered by Key to form log(n) time complexity

void Init_func_symtab(T_BTNode** rootPtr){
    T_BTNode* temp;
    temp = BT_insert(rootPtr,"int2float");
    temp->declared = true;
    add_type_node(&temp->rets, FLOAT);
    add_type_node(&temp->args, INT);

    temp = BT_insert(rootPtr,"float2int"); 
    temp->declared = true;
    add_type_node(&temp->rets, INT);
    add_type_node(&temp->args, FLOAT);

    temp = BT_insert(rootPtr,"chr");
    temp->declared = true;
    add_type_node(&temp->rets,QM_STRING);
    add_type_node(&temp->rets,INT);
    add_type_node(&temp->args,INT);

    temp = BT_insert(rootPtr,"inputi"); 
    temp->declared = true;
    add_type_node(&temp->rets,INT);
    add_type_node(&temp->rets,INT);

    temp = BT_insert(rootPtr,"inputf"); 
    temp->declared = true;
    add_type_node(&temp->rets,FLOAT);
    add_type_node(&temp->rets,INT);

    temp = BT_insert(rootPtr,"inputs"); 
    temp->declared = true;
    add_type_node(&temp->rets,QM_STRING);
    add_type_node(&temp->rets,INT);

    temp = BT_insert(rootPtr,"ord");
    temp->declared = true;
    add_type_node(&temp->rets, INT);
    add_type_node(&temp->rets, INT);
    add_type_node(&temp->args, QM_STRING);
    add_type_node(&temp->args, INT);

    temp = BT_insert(rootPtr,"len"); 
    temp->declared = true;
    add_type_node(&temp->rets,INT);
    add_type_node(&temp->args,QM_STRING);

    temp = BT_insert(rootPtr,"print");
    temp->args = NULL;
    temp->rets = NULL;
    temp->declared = true;

    temp = BT_insert(rootPtr,"substr"); 
    temp->declared = true;
    add_type_node(&temp->rets, QM_STRING);
    add_type_node(&temp->rets, INT);
    add_type_node(&temp->args, QM_STRING);
    add_type_node(&temp->args, INT);
    add_type_node(&temp->args, INT);

    sa_data.inside_func = temp; //
    code_gen_builtin();
    sa_data.inside_func = NULL;  
}

//freeing all allocated resources
void free_resources(){
    dispose_type_list(&sa_data.left_side_types);
    dispose_type_list(&sa_data.right_side_types);
    dispose_left_side_id(&sa_data.left_side_ids);
    free(TOKEN_DATA);
    free(sa_data.token);
    sa_stack_empty(&sa_stack);
    free(sa_data.func_code.output);
    free(sa_data.main_code.output);
    SPA_dispose();
}

//exiting with correct exit value (printing IFJcode20 there were no errors found)
void free_and_exit(int exit_code)
{
    if(exit_code != 0){
        switch(exit_code){
            case 1:
                fprintf(stderr, "\n#LEXICAL ANALYSIS ERROR# (1)\n");
                break;
            case 2:
                fprintf(stderr, "\n#SYNTAX ANALYSIS ERROR# (2)\n");
                break;
            case 3:
                fprintf(stderr, "\n#DEFINITION ERROR# (3)\n");
                break;
            case -1:
                fprintf(stderr, "\n#END OF FILE ERROR# (-1)\n");
                break;
            case 6:
                fprintf(stderr, "\n#PARAMETER ERROR# (6)\n");
                break;
            case 5:
                fprintf(stderr,"\n#TYPE ERROR# (5)\n");
                break;
            case 7:
                fprintf(stderr,"\n#SEMANTIC ERROR# (7)\n");
                break;
            case 9:
                fprintf(stderr, "\n#ZERO DIVISON ERROR# (9)\n");
                break;
            case 99:
                fprintf(stderr, "\n#INTERNAL ERROR# (99)\n ");
                break;
        }
    }  else {
        printf(".IFJcode20\n");
        printf("DEFVAR GF@exp\n");
        printf("DEFVAR GF@exp1\n");
        printf("DEFVAR GF@err\n");
        printf("JUMP main\n");
        printf("LABEL divzero_err\n");
        printf("EXIT int@9\n");
        printf("%s", sa_data.func_code.output);
        printf("LABEL main\n");
        printf("CREATEFRAME\n");
        printf("%s", sa_data.main_code.output);
        printf("\n");
        printf("LABEL $endmain\n");
        fprintf(stderr, "#ALL PASSED!# (0)\n\n");
    } 
    free_resources();
    exit(exit_code);
}

// function for scanner calls
// return value from scanner is not 0 the program exits with error code 1
// if there is an EOF found before all block are closed the program exits with error code 2
// if there is a problem with data allocation the program exits with error code 99
void get_token()
{
    free(TOKEN_DATA);
    TOKEN_DATA = NULL;
    TOKEN_DATA = malloc(sizeof(char)*9);
    if(TOKEN_DATA == NULL)
    {
        free_and_exit(INTERNAL_ERR);
    }
    int retval = scanner(&(TOKEN_TYPE), &(TOKEN_DATA));
    
    if(retval != ALL_PASS){
        free_and_exit(retval);
    }
    if(TOKEN_TYPE == EOIF && (sa_stack->item_index > 0))
        free_and_exit(SYNTAX_ERR);
    
}


// functions according to LL table
// if an error is found, all resources are freed and
// the program exits with errors exit value
// also there are parts of code generation scattered
// in these functions

void prologue()
{
    skip_endline();

    if (cmp_type(KW_PACKAGE)){
        if  (get_token_cmp_type(KW_MAIN)){
            if(get_token_cmp_type(END_LINE)){
                declaration_list();
            } else free_and_exit(SYNTAX_ERR);
        } else {
            free_and_exit(SYNTAX_ERR);
        }
    } else free_and_exit(SYNTAX_ERR);

    if (sa_data.main_found != true){
        free_and_exit(DEF_ERR);
    }
}

void declaration_list()
{
    skip_endline();
    sa_data.return_found = false;
    if (cmp_type(KW_FUNC)){
        sa_stack_push(&sa_stack, 0);
        if (get_token_cmp_type(ID)){

            if ((sa_data.inside_func = BT_search(sa_data.func_symtab,NULL,TOKEN_DATA)) == NULL)
            {
                sa_data.inside_func = BT_insert(FUNC_SYMTAB, TOKEN_DATA);         
            } else {
                if (sa_data.inside_func->declared == true) {
                    free_and_exit(DEF_ERR);
                }
            }
            code_insert("LABEL ");
            code_insert(TOKEN_DATA);
            code_insert("\n");
            code_insert("CREATEFRAME\n");
            sa_data.inside_func->declared = true;
            if(get_token_cmp_type(L_PARENTH)){
                param_list();
            } else free_and_exit(SYNTAX_ERR);
            
        } else if (cmp_type(KW_MAIN) && sa_data.main_found == false){ 

            sa_data.main_found = true;
            sa_data.inside_func = BT_insert(FUNC_SYMTAB, TOKEN_DATA); 
            sa_data.inside_func->declared =true;        
            if (get_token_cmp_type(L_PARENTH)){
                if (get_token_cmp_type(R_PARENTH)){
                    if (get_token_cmp_type(START_BLOCK)){
                        if (get_token_cmp_type(END_LINE)){
                            statement_list();
                        } else free_and_exit(SYNTAX_ERR);
                        
                    } else if (cmp_type(L_PARENTH)){
                        
                        if (get_token_cmp_type(R_PARENTH)){
                            if (get_token_cmp_type(START_BLOCK)){
                                if (get_token_cmp_type(END_LINE)){
                                    statement_list();
                                } else free_and_exit(SYNTAX_ERR);
                            } else free_and_exit(SYNTAX_ERR);
                        } else {
                            type();
                            free_and_exit(PAR_ERR);  
                        } 

                    } else free_and_exit(SYNTAX_ERR);
                } else {
                    if (cmp_type(ID)) 
                    { 
                        free_and_exit(PAR_ERR); 
                    } else free_and_exit(SYNTAX_ERR); 
                }
            } else free_and_exit(SYNTAX_ERR);
                   
                   
        } else if (TOKEN_TYPE == KW_MAIN && sa_data.main_found == true) {
            free_and_exit(DEF_ERR);
        }
    } else if(cmp_type(EOIF)){
        FUNC_CURRENT = BT_search(sa_data.func_symtab, NULL, "print");
        if(FUNC_CURRENT != NULL)
        FUNC_CURRENT->rets = NULL;
        BTInorder(*FUNC_SYMTAB,check_func_declaration);
        if (!sa_data.main_found)
            free_and_exit(DEF_ERR);
        
        free_and_exit(ALL_PASS);
    } else {
        free_and_exit(SYNTAX_ERR);
    }
}



void param_list(){
    if (get_token_cmp_type(R_PARENTH)){
        if(sa_data.inside_func->used == true) 
        {
            cmp_multiple_types(sa_data.inside_func->args, NULL,true);
        }
        if (get_token_cmp_type(L_PARENTH)){
            return_list();
        }else if (cmp_type(START_BLOCK)){
            if (get_token_cmp_type(END_LINE)){
                    statement_list();
            } else free_and_exit(SYNTAX_ERR);
        } else free_and_exit(SYNTAX_ERR);
    } else if (cmp_type(ID)){
        code_insert("DEFVAR ");
        code_insert("TF@");
        code_insert(TOKEN_DATA);
        code_insert("1");
        code_insert("\n");

        VAR_CURRENT = BT_search(sa_stack, sa_stack->next, TOKEN_DATA);
        if(VAR_CURRENT != NULL){
            free_and_exit(DEF_ERR);
        }
        VAR_CURRENT = BT_insert(&(sa_stack->local_symtab), TOKEN_DATA);
        VAR_CURRENT->declared = true;
        get_token();
        VAR_CURRENT->type = type();
        add_type_node(&(sa_data.right_side_types),VAR_CURRENT->type);
        add_id_node(&(sa_data.left_side_ids), VAR_CURRENT->Key, 1);
        param();
    } else free_and_exit(SYNTAX_ERR);
}

void param(){
    if(get_token_cmp_type(COMMA)){ 

        if(get_token_cmp_type(ID)){ 
            code_insert("DEFVAR ");
            code_insert("TF@");
            code_insert(TOKEN_DATA);
            code_insert("1");
            code_insert("\n");

            VAR_CURRENT = BT_search(sa_stack, sa_stack->next, TOKEN_DATA);
            if(VAR_CURRENT != NULL){
                free_and_exit(DEF_ERR);
            }
            VAR_CURRENT = BT_insert(&(sa_stack->local_symtab), TOKEN_DATA);
            VAR_CURRENT->declared = true;
            add_id_node(&(sa_data.left_side_ids), VAR_CURRENT->Key, 1);
            get_token();
            VAR_CURRENT->type = type();
            add_type_node(&(sa_data.right_side_types),VAR_CURRENT->type);
            param();

        }else if (cmp_type(R_PARENTH)){
            if(sa_data.inside_func->used == true) 
            {
                cmp_multiple_types(sa_data.inside_func->args, sa_data.right_side_types,true);
                dispose_type_list(&(sa_data.right_side_types));
            } else {
                sa_data.inside_func->args = sa_data.right_side_types;
            }                    
            sa_data.right_side_types = NULL;
            T_left_side_id_list* temp;
            while(sa_data.left_side_ids != NULL){
                code_insert("POPS TF@");
                temp = pop_end_id(&(sa_data.left_side_ids));
                code_insert(temp->id);
                int_code_insert(temp->depth);
                code_insert("\n");
                free(temp);
            }

            if(get_token_cmp_type(L_PARENTH)){ 
                return_list();

            } else if (cmp_type(START_BLOCK)){
                if(get_token_cmp_type(END_LINE)){
                    statement_list();
                } else free_and_exit(SYNTAX_ERR);
            } else free_and_exit(SYNTAX_ERR);

        } else free_and_exit(SYNTAX_ERR);
        
    } else if (cmp_type(R_PARENTH)){ 
        T_left_side_id_list* temp;
        while(sa_data.left_side_ids != NULL){
            code_insert("POPS TF@");
            temp = pop_end_id(&(sa_data.left_side_ids));
            code_insert(temp->id);
            int_code_insert(temp->depth);
            code_insert("\n");
            free(temp);
        }

        if(get_token_cmp_type(L_PARENTH)){
            if(sa_data.inside_func->used == true) 
            {
                cmp_multiple_types(sa_data.inside_func->args, sa_data.right_side_types,true);
                dispose_type_list(&(sa_data.right_side_types));
            } else {
                sa_data.inside_func->args = sa_data.right_side_types;
               
            }
            sa_data.right_side_types = NULL;
            
            return_list();
        } else if (cmp_type(START_BLOCK)){

            if(get_token_cmp_type(END_LINE)){
                if(sa_data.inside_func->used == true) 
                {
                    cmp_multiple_types(sa_data.inside_func->args, sa_data.right_side_types,true);
                    dispose_type_list(&(sa_data.right_side_types));
                } else {
                    sa_data.inside_func->args = sa_data.right_side_types;
                }
                sa_data.right_side_types = NULL;
                statement_list();
            } else free_and_exit(SYNTAX_ERR);

        }else free_and_exit(SYNTAX_ERR);
        
    } else free_and_exit(SYNTAX_ERR);

}

int type(){
    if (cmp_type(KW_INT)){
        return INT;
    } else if (cmp_type(KW_STRING)){
        return QM_STRING;
    } else if (cmp_type(KW_FLOAT64)){
        return FLOAT;
    } else {
        free_and_exit(SYNTAX_ERR);
    }
    return -1;
}

void return_list(){
    
    if(get_token_cmp_type(R_PARENTH)){
        if (get_token_cmp_type(START_BLOCK)){
            if (get_token_cmp_type(END_LINE)){
                if(sa_data.inside_func->used == true) 
                {
                    cmp_multiple_types(sa_data.inside_func->rets, sa_data.left_side_types,true);
                    dispose_type_list(&(sa_data.left_side_types));
                } else {
                    sa_data.inside_func->rets = sa_data.left_side_types;
                }
                sa_data.left_side_types = NULL;

                statement_list(); 
            } else free_and_exit(SYNTAX_ERR);
        } else free_and_exit(SYNTAX_ERR);
    } else {
        code_insert("DEFVAR TF@$retval");
        sa_data.retval_counter++;
        VAR_CURRENT = BT_insert(LOCAL_SYMTAB, "$retval");
        VAR_CURRENT->type = ID;
        int_code_insert(sa_data.retval_counter);
        int_code_insert(1);
        code_insert("\n");
        code_insert("MOVE TF@$retval");
        int_code_insert(sa_data.retval_counter);
        int_code_insert(1);
        code_insert(" nil@nil");
        code_insert("\n");
        add_type_node(&(sa_data.left_side_types),type());    
        return_vals();
    }
}

void return_vals(){
    if(get_token_cmp_type(COMMA)){ 
        if(get_token_cmp_type(R_PARENTH)){
            if(get_token_cmp_type(START_BLOCK)){
                if(get_token_cmp_type(END_LINE)){
                    if(sa_data.inside_func->used == true) 
                    {
                        cmp_multiple_types(sa_data.inside_func->rets, sa_data.left_side_types,true);
                        dispose_type_list(&(sa_data.left_side_types));
                    }  else {
                        sa_data.inside_func->rets = sa_data.left_side_types;
                    }
                    sa_data.left_side_types = NULL;
                    statement_list();
                } else free_and_exit(SYNTAX_ERR);
            } else free_and_exit(SYNTAX_ERR);

        } else {
            code_insert("DEFVAR TF@$retval");
            sa_data.retval_counter++;
            VAR_CURRENT = BT_insert(LOCAL_SYMTAB, "$retval");
            VAR_CURRENT->type = ID;
            int_code_insert(sa_data.retval_counter);
            int_code_insert(1);
            code_insert("\n");
            code_insert("MOVE TF@$retval");
            int_code_insert(sa_data.retval_counter);
            int_code_insert(1);
            code_insert(" nil@nil\n");
            add_type_node(&(sa_data.left_side_types),type());    
            return_vals();
        }

    } else if(cmp_type(R_PARENTH)){

        if(get_token_cmp_type(START_BLOCK)){

            if(get_token_cmp_type(END_LINE)){
                if(sa_data.inside_func->used == true) 
                    {
                        cmp_multiple_types(sa_data.inside_func->rets, sa_data.left_side_types,true);
                        dispose_type_list(&(sa_data.left_side_types));
                    }  else {
                        sa_data.inside_func->rets = sa_data.left_side_types;
                    }
                    sa_data.left_side_types = NULL;
                statement_list();
            } else free_and_exit(SYNTAX_ERR);
        } else free_and_exit(SYNTAX_ERR);

    } else free_and_exit(SYNTAX_ERR);
}

void statement_list(){
    sa_data.retval_counter = -1;
    skip_endline();
    if(cmp_type(END_BLOCK)){ 
            if (sa_stack->item_index < 2){
                if(get_token_cmp_type(END_LINE)){
                    if(strcmp(sa_data.inside_func->Key,"main") != 0){
                        if(sa_data.return_found == false){
                            code_insert("RETURN\n");
                            if (sa_data.inside_func->rets != NULL)
                            {
                                free_and_exit(PAR_ERR);   
                            }
                        }
                    } 
                    sa_stack_pop(&sa_stack);
                    declaration_list();
                } else{
                    free_and_exit(SYNTAX_ERR);
                } 
            } else {
                if (sa_stack->type == B_IF){
                    int temp = sa_stack->label_index;
                    code_insert("JUMP endif");
                    int_code_insert(sa_stack->label_index);
                    code_insert("\n");
                    code_insert("LABEL else");
                    int_code_insert(sa_stack->label_index);
                    code_insert("\n");
                    sa_stack_pop(&sa_stack);
                    if(get_token_cmp_type(KW_ELSE)){
                        if(get_token_cmp_type(START_BLOCK)){         
                            if(get_token_cmp_type(END_LINE)){
                                sa_stack_push(&sa_stack, B_ELSE);
                                sa_stack->label_index = temp;
                                statement_list();
                            } else free_and_exit(SYNTAX_ERR); 
                        } else free_and_exit(SYNTAX_ERR);    
                    } else free_and_exit(SYNTAX_ERR);

                } else if (sa_stack->type == B_FOR){
                    
                    sa_stack_pop(&sa_stack);
                    code_gen_clear_frame();
                    code_insert("JUMP for_inc");
                    int_code_insert(sa_stack->label_index);
                    code_insert("\n");
                    
                    code_insert("LABEL for_end");
                    int_code_insert(sa_stack->label_index);
                    code_insert("\n");
                    sa_stack_pop(&sa_stack);
                    if(get_token_cmp_type(END_LINE)){
                        statement_list();
                    } else free_and_exit(SYNTAX_ERR);

                } else if(sa_stack->type == B_ELSE){
                    code_insert("LABEL endif");
                    int_code_insert(sa_stack->label_index);
                    code_insert("\n");
                    sa_stack_pop(&sa_stack);
                    if(get_token_cmp_type(END_LINE)){
                        statement_list();
                    } else free_and_exit(SYNTAX_ERR);
                } else free_and_exit(SYNTAX_ERR);
            }
    } else if(cmp_type(START_BLOCK)){      
        free_and_exit(SYNTAX_ERR);
    } else {                                                            
        statement();
        statement_list();    
    }
}

void statement(){
   
    dispose_left_side_id(&(sa_data.left_side_ids));
    skip_endline();

    if (cmp_type(ID) || cmp_type(UNDERSCORE)){                                       

        char* c = malloc(sizeof(char)*strlen(TOKEN_DATA));
        if(c == NULL)
            free_and_exit(INTERNAL_ERR);
        strcpy(c, TOKEN_DATA);
        VAR_CURRENT = BT_search(sa_stack, sa_data.func_symtab,c);
        if (get_token_cmp_type(L_PARENTH)){                            
            if (VAR_CURRENT !=NULL)
            {
                free(c);
                free_and_exit(DEF_ERR);
            }
            FUNC_CURRENT  = BT_search_and_insert(sa_data.func_symtab, NULL,c,FUNC_SYMTAB);
            free(c);
            if(get_token_cmp_type(R_PARENTH) && (strcmp(FUNC_CURRENT->Key,"print") != 0 )){
                code_insert("PUSHFRAME\nCREATEFRAME\nCALL ");
                code_insert(FUNC_CURRENT->Key);
                code_insert("\n");

                if (get_token_cmp_type(END_LINE)){
                    if(FUNC_CURRENT->declared == true){
                        cmp_multiple_types(FUNC_CURRENT->args, NULL, true);
                        cmp_multiple_types(FUNC_CURRENT->rets, NULL, true);
                    } else {
                        FUNC_CURRENT->rets = NULL;
                        FUNC_CURRENT->args = NULL;
                        FUNC_CURRENT->used = true;
                    }
                } else free_and_exit(SYNTAX_ERR);
            } else if (strcmp(FUNC_CURRENT->Key,"print") == 0){
                if(cmp_type(R_PARENTH)){
            
                } else if(cmp_type(ID)){
                    if((VAR_CURRENT = BT_search(sa_stack, sa_data.func_symtab, TOKEN_DATA)) == NULL){
                        free_and_exit(DEF_ERR);
                    } else {
                        if(VAR_CURRENT->depth == sa_stack->item_index){
                            code_insert("WRITE ");
                            generate_type(ID, TOKEN_DATA,VAR_CURRENT->depth);
                            code_insert("\n");
                        } else {
                            code_insert("WRITE ");
                            code_insert("TF@");
                            code_insert(TOKEN_DATA);
                            int_code_insert(VAR_CURRENT->depth);
                            code_insert("\n");
                        }
                    }
                    get_token();
                } else {
                    code_insert("WRITE ");
                    generate_type(TOKEN_TYPE, TOKEN_DATA, 0);
                    code_insert("\n");
                    get_token();
                }
    
                while(!(cmp_type(R_PARENTH))){
                    if(cmp_type(COMMA)){
                        get_token();
                        if(cmp_type(ID)){
                            if((VAR_CURRENT = BT_search(sa_stack, sa_data.func_symtab, TOKEN_DATA)) == NULL){
                                free_and_exit(DEF_ERR);
                            } else {
                                if(VAR_CURRENT->depth == sa_stack->item_index){
                                    code_insert("WRITE ");
                                    generate_type(ID, TOKEN_DATA,VAR_CURRENT->depth);
                                    code_insert("\n");
                                } else {
                                    code_insert("WRITE ");
                                 code_insert("TF@");
                                    code_insert(TOKEN_DATA);
                                    int_code_insert(VAR_CURRENT->depth);
                                    code_insert("\n");
                                }
                            }
                        } else {
                            code_insert("WRITE ");
                            generate_type(TOKEN_TYPE, TOKEN_DATA, 0);
                            code_insert("\n");
                        }
                    }else free_and_exit(SYNTAX_ERR);
                    get_token();
                }
                if(get_token_cmp_type(END_LINE)){

                }else free_and_exit(SYNTAX_ERR);
                
            }else{
                arg_list();
                code_insert("PUSHFRAME\nCREATEFRAME\nCALL ");
                code_insert(FUNC_CURRENT->Key);
                code_insert("\n");
                code_insert("POPFRAME\n");
            }
        } else{

            VAR_CURRENT = BT_search_and_insert(sa_stack, sa_data.func_symtab, c,LOCAL_SYMTAB);
            free(c);
            FUNC_CURRENT = NULL;  
            statement_id();
            if(FUNC_CURRENT != NULL){
            }

        }

    } else if (cmp_type(KW_IF)){ 
        sa_data.label_counter++;
        sa_stack_push(&sa_stack, B_IF);
        if(!(get_token_cmp_type(START_BLOCK))){
            if(expression(false, START_BLOCK) == UNDEFINED)
            {
                free_and_exit(SEM_ERR);
            }
            code_insert("JUMPIFEQ else");
            int_code_insert(sa_stack->label_index); 
            code_insert(" GF@exp bool@false\n");
            if(sa_data.cmp_used == false){
                free_and_exit(TYPE_ERR);
            }
            sa_data.cmp_used = false;
            if(get_token_cmp_type(END_LINE)){
                statement_list();
            } else free_and_exit(SYNTAX_ERR);
        } else free_and_exit(SYNTAX_ERR);

    } else if (cmp_type(KW_FOR)){
        sa_data.label_counter++;
        sa_stack_push(&sa_stack, 0); 
        if (get_token_cmp_type(ID)){

            VAR_CURRENT = BT_search_and_insert(sa_stack, sa_data.func_symtab,TOKEN_DATA, LOCAL_SYMTAB);
            if(get_token_cmp_type(DECLARATION)){

                if (VAR_CURRENT->declared == true)
                {
                    VAR_CURRENT = BT_insert(LOCAL_SYMTAB,VAR_CURRENT->Key);
                }
                VAR_CURRENT->declared = true;
                get_token();
                int temp_type = expression(true, SEMICOLON);
                if (temp_type == UNDEFINED) {
                   free_and_exit(DEF_ERR);
                } else {
                   VAR_CURRENT->type = temp_type;
                   code_insert("DEFVAR TF@");
                   code_insert(VAR_CURRENT->Key);
                   int_code_insert(VAR_CURRENT->depth);
                   code_insert("\n");
                   code_insert("MOVE TF@");
                   code_insert(VAR_CURRENT->Key);
                   int_code_insert(VAR_CURRENT->depth);
                   code_insert(" GF@exp\n");
                }               
            } else free_and_exit(SYNTAX_ERR);
            
        } else if (cmp_type(SEMICOLON)){     
        }else free_and_exit(SYNTAX_ERR);

        code_insert("LABEL for_if");
        int_code_insert(sa_stack->label_index);
        code_insert("\n");
        if(cmp_type(END_LINE)){
            free_and_exit(SYNTAX_ERR);
        }
        get_token();
        sa_stack_push(&sa_stack, 0);
        if(expression(false, SEMICOLON) == UNDEFINED){
            free_and_exit(SEM_ERR);
        }
        if(sa_data.cmp_used == false){
            free_and_exit(TYPE_ERR);
        }
        sa_data.cmp_used = false;
        sa_stack_pop(&sa_stack);
        code_insert("JUMPIFEQ for_end");
        int_code_insert(sa_stack->label_index);
        code_insert(" GF@exp bool@false\n");
        code_insert("JUMP for_body");
        int_code_insert(sa_stack->label_index);
        code_insert("\n");
        if (cmp_type(END_LINE))
        {
            free_and_exit(SYNTAX_ERR);
        }
        code_insert("LABEL for_inc");
        int_code_insert(sa_stack->label_index);
        code_insert("\n");
        if (get_token_cmp_type(ID)){
            
            VAR_CURRENT = BT_search(sa_stack,sa_data.func_symtab,TOKEN_DATA);
            if (VAR_CURRENT == NULL){
                free_and_exit(DEF_ERR);
            }
            if (get_token_cmp_type(EQUAL)){
                get_token();
                
                if ((VAR_CURRENT->type =expression(true, START_BLOCK)) == UNDEFINED) 
                {
                    free_and_exit(SEM_ERR);
                }
                if(cmp_type(END_LINE)){
                    free_and_exit(SYNTAX_ERR);
                }
                code_insert("MOVE TF@");
                code_insert(VAR_CURRENT->Key);
                int_code_insert(VAR_CURRENT->depth);
                code_insert(" GF@exp\n");
                
            
            } else free_and_exit(SYNTAX_ERR);

        }  else if (cmp_type(START_BLOCK)){
        } else free_and_exit(SYNTAX_ERR);
        code_insert("JUMP for_if");
        int_code_insert(sa_stack->label_index);
        code_insert("\n");
        code_insert("LABEL for_body");
        int_code_insert(sa_stack->label_index);
        code_insert("\n");
        sa_stack_push(&sa_stack, B_FOR);
        if(get_token_cmp_type(END_LINE)){
            statement_list();
        } else free_and_exit(SYNTAX_ERR);      
    } else if(cmp_type(KW_RETURN)){
        sa_data.return_found = true;
        get_token();
        if(strcmp(sa_data.inside_func->Key,"main") == 0)
        {
            code_insert("JUMP $endmain\n");
            if (!cmp_type(END_LINE))
            {
                free_and_exit(PAR_ERR);
            }
        } else {
        
            sa_data.inside_return = true;
            if(cmp_type(END_LINE)){
                cmp_multiple_types(sa_data.inside_func->rets, NULL,true);
            } else {
                int temp_type = expression(true,COMMA);
                if (temp_type != UNDEFINED)
                {
                    add_type_node(&(sa_data.right_side_types) ,temp_type);
                } else free_and_exit(SEM_ERR);
                sa_data.retval_counter++;
                VAR_CURRENT = BT_insert(LOCAL_SYMTAB, "$retval");
                add_id_node(&(sa_data.left_side_ids), VAR_CURRENT->Key,VAR_CURRENT->depth);
            }
        }
        expression_list();
        cmp_multiple_types(sa_data.inside_func->rets, sa_data.right_side_types,true);
        dispose_type_list(&(sa_data.right_side_types));
        sa_data.right_side_types = NULL;
        sa_data.inside_return = false;
        sa_data.retval_counter = 0;
        if(strcmp(sa_data.inside_func->Key,"main") != 0)
        {
            code_insert("RETURN\n"); 
        }
    }else {                                                            
        free_and_exit(SYNTAX_ERR);
    }
    if (FUNC_CURRENT != NULL)
    {
        FUNC_CURRENT->used = true;
    }
}

void statement_id(){
    if(cmp_type(DECLARATION)){
        if(VAR_CURRENT->declared == true){
            if(VAR_CURRENT == BT_search(sa_stack, sa_stack->next, VAR_CURRENT->Key))
            {
                free_and_exit(DEF_ERR);
            } else {
                VAR_CURRENT = BT_insert(LOCAL_SYMTAB, VAR_CURRENT->Key);
            }
        }
        code_insert("DEFVAR ");
        generate_type(ID, VAR_CURRENT->Key, VAR_CURRENT->depth);
        code_insert("\n");
        get_token();
 
        int temp_type = expression(true, END_LINE);
        if(temp_type == UNDEFINED)
        {
            free_and_exit(DEF_ERR); 
        } else {
            VAR_CURRENT->type = temp_type;
            code_insert("MOVE TF@");
            code_insert(VAR_CURRENT->Key);
            int_code_insert(VAR_CURRENT->depth);
            code_insert(" GF@exp");
            code_insert("\n");
        }   
        VAR_CURRENT->declared = true;
    } else if (cmp_type(EQUAL)){
        if(VAR_CURRENT->declared == false) free_and_exit(DEF_ERR);
        add_type_node(&(sa_data.left_side_types), VAR_CURRENT->type); 
        get_token();
        int temp_type = expression(true,END_LINE);
        if (temp_type == UNDEFINED)
        {
    
            if (FUNC_CURRENT->declared || FUNC_CURRENT->used)
            {
                cmp_multiple_types(sa_data.left_side_types,FUNC_CURRENT->rets,true);
                dispose_type_list(&(sa_data.left_side_types));
            } else {
                FUNC_CURRENT->rets = sa_data.left_side_types;
                sa_data.left_side_types = NULL;
            }

            if (strcmp(FUNC_CURRENT->Key, "print") != 0)
            {
                code_insert("PUSHFRAME\nCREATEFRAME\nCALL ");
                code_insert(FUNC_CURRENT->Key);
                code_insert("\n");


                if (VAR_CURRENT->type != UNDERSCORE)
                {
                    code_insert("MOVE LF@");
                    code_insert(VAR_CURRENT->Key);
                    int_code_insert(VAR_CURRENT->depth);              
                    code_insert(" TF@$retval01\n");
                    
                }
                code_insert("POPFRAME\n");
            }
        } else if ((temp_type != VAR_CURRENT->type) && (VAR_CURRENT->type != UNDERSCORE)){
            free_and_exit(TYPE_ERR);
        } else if(VAR_CURRENT->type != UNDERSCORE){ 
            code_insert("MOVE ");
            generate_type(ID,VAR_CURRENT->Key,VAR_CURRENT->depth);
            code_insert(" GF@exp\n");
        }

        dispose_type_list(&sa_data.left_side_types);
        VAR_CURRENT->used = true;
    }else if(cmp_type(COMMA)){
        leftside_id();
        cmp_multiple_types(sa_data.left_side_types,sa_data.right_side_types,false);
        dispose_type_list(&(sa_data.left_side_types));
        dispose_type_list(&(sa_data.right_side_types));
    } else free_and_exit(SYNTAX_ERR);
}

// extension of statement_id for multiple ids on the left side of equation
void leftside_id(){
    if(VAR_CURRENT->declared == false)
    {
        free_and_exit(DEF_ERR);
    }
    if(get_token_cmp_type(ID) || cmp_type(UNDERSCORE)){ 
        add_id_node(&(sa_data.left_side_ids),VAR_CURRENT->Key,VAR_CURRENT->depth);
        add_type_node(&(sa_data.left_side_types),VAR_CURRENT->type);
        VAR_CURRENT = BT_search_and_insert(sa_stack,sa_data.func_symtab,TOKEN_DATA,LOCAL_SYMTAB);
        if (get_token_cmp_type(EQUAL)){
            add_id_node(&(sa_data.left_side_ids),VAR_CURRENT->Key,VAR_CURRENT->depth);
            add_type_node(&(sa_data.left_side_types),VAR_CURRENT->type);
            get_token();
            int temp_type;
            if ((temp_type = expression(true,COMMA)) != UNDEFINED)
            {
                add_type_node(&(sa_data.right_side_types),temp_type);
                
                T_left_side_id_list* temp = pop_id(&sa_data.left_side_ids);
                if (temp == NULL)
                    free_and_exit(SEM_ERR);
                if (strcmp(temp->id,"_") != 0)
                {
                    code_insert("MOVE TF@");
                    code_insert(temp->id);
                    int_code_insert(temp->depth);
                    code_insert(" GF@exp");
                    code_insert("\n");
                }
                free(temp);
                expression_list();
            } else {
                code_insert("PUSHFRAME\nCREATEFRAME\nCALL ");
                code_insert(FUNC_CURRENT->Key);
                code_insert("\n");
                T_type_list* tmp = sa_data.left_side_types;
                while(tmp!=NULL){
                    T_left_side_id_list* temp = pop_id(&sa_data.left_side_ids);
                    sa_data.retval_counter++;
                    if (temp == NULL)
                        free_and_exit(SEM_ERR); 
                    if(strcmp(temp->id, "_") != 0){
                      code_insert("MOVE LF@");
                        code_insert(temp->id);
                        int_code_insert(temp->depth);
                        code_insert(" TF@$retval");
                        int_code_insert(sa_data.retval_counter);
                        int_code_insert(1);
                        code_insert("\n");
                    }
                    free(temp);
                    tmp = tmp->next;
                }
                code_insert("POPFRAME\n");

                if (FUNC_CURRENT->declared || FUNC_CURRENT->used) 
                {
                    cmp_multiple_types(sa_data.left_side_types,FUNC_CURRENT->rets,true);
                    dispose_type_list(&(sa_data.left_side_types));
                } else {
                    FUNC_CURRENT->rets = sa_data.left_side_types;
                    sa_data.left_side_types = NULL;
                }
            }
        } else if (cmp_type(COMMA)) {
            leftside_id();
        } else free_and_exit(SYNTAX_ERR);
    }else free_and_exit(SYNTAX_ERR);
}

void arg_list(){
    if (cmp_type(ID)){  
        if ((sa_data.previous_var_node = BT_search(sa_stack,sa_data.func_symtab,TOKEN_DATA)) == NULL)
        {
            free_and_exit(DEF_ERR);
        }    
        add_type_node(&(sa_data.right_side_types),sa_data.previous_var_node->type); 
        
        code_insert("PUSHS TF@");
        code_insert(sa_data.previous_var_node->Key);
        int_code_insert(sa_data.previous_var_node->depth);
        code_insert("\n");
        arg();

    } else if (cmp_type(R_PARENTH)){   
        if(get_token_cmp_type(END_LINE)) {
           if (FUNC_CURRENT->declared || FUNC_CURRENT->used) {
               cmp_multiple_types(FUNC_CURRENT->args,sa_data.right_side_types,true);
               dispose_type_list(&sa_data.right_side_types);
           } else {
               FUNC_CURRENT->args = sa_data.right_side_types;
           }
           sa_data.right_side_types = NULL;
        } else free_and_exit(SYNTAX_ERR);  
    } else if (cmp_type(UNDERSCORE)){
        free_and_exit(SEM_ERR);
    }else {
        value();
        arg();
    }
}
void arg(){

    if(get_token_cmp_type(COMMA)){
        if(get_token_cmp_type(ID)){
            if ((sa_data.previous_var_node = BT_search(sa_stack,sa_data.func_symtab,TOKEN_DATA)) == NULL)
            {
                free_and_exit(DEF_ERR); 
            } 
            add_type_node(&(sa_data.right_side_types) , sa_data.previous_var_node->type);
            
            code_insert("PUSHS ");
            generate_type(ID,sa_data.previous_var_node->Key,sa_data.previous_var_node->depth);
            code_insert("\n");  
        } else if (cmp_type(UNDERSCORE)) {
            free_and_exit(SEM_ERR);
        } else{
            value();
        }
        arg();
    } else if (cmp_type(R_PARENTH)){
        
        if (get_token_cmp_type(END_LINE)){
            if (FUNC_CURRENT->declared || FUNC_CURRENT->used) { 
                cmp_multiple_types(FUNC_CURRENT->args,sa_data.right_side_types,true);
                dispose_type_list(&sa_data.right_side_types);
           } else {
               FUNC_CURRENT->args = sa_data.right_side_types;
           }
           sa_data.right_side_types = NULL;
        } else free_and_exit(SYNTAX_ERR); 
    } else free_and_exit(SYNTAX_ERR); 
}

void value(){

    if(cmp_type(INT)){
    } else if (cmp_type(FLOAT)){
    } else if (cmp_type(QM_STRING)){
    } else free_and_exit(SYNTAX_ERR); 
   
    code_insert("PUSHS ");
    generate_type(TOKEN_TYPE, TOKEN_DATA, 0);
    code_insert("\n");
    
    add_type_node(&(sa_data.right_side_types),TOKEN_TYPE); 
}

//function for multiple expresions seperated by commas
void expression_list(){
    if(sa_data.inside_return == true && sa_data.retval_counter == 0){
        T_left_side_id_list* temp = pop_id(&sa_data.left_side_ids);
        if (temp == NULL){
            free_and_exit(SEM_ERR);
        }
        if (strcmp(temp->id, "_") != 0)
        {
            if(sa_data.inside_return == false){
                if (temp == NULL)
                    free_and_exit(SEM_ERR);
                code_insert(temp->id);
                int_code_insert(temp->depth);
                code_insert(" GF@exp");
                code_insert("\n");
      
            } else {

                code_insert("MOVE TF@$retval");
                int_code_insert(sa_data.retval_counter);
                int_code_insert(1);
                code_insert(" GF@exp");
                code_insert("\n");
            }
        } 
        sa_data.retval_counter++;
        free(temp);
    }
    
    if(cmp_type(COMMA)){
        get_token();
        
        VAR_CURRENT = BT_insert(LOCAL_SYMTAB, "$retval");
        add_id_node(&(sa_data.left_side_ids), VAR_CURRENT->Key,VAR_CURRENT->depth); 
        int temp_type; 
        if ((temp_type = expression(true,COMMA)) != UNDEFINED)
        {
            add_type_node(&(sa_data.right_side_types),temp_type);
            if (sa_data.left_side_ids != NULL) { 
                
                T_left_side_id_list* temp = pop_id(&sa_data.left_side_ids);
                if (temp == NULL)
                    free_and_exit(SEM_ERR);
                code_insert("MOVE ");

                if(sa_data.inside_return == false){
                    if(strcmp(temp->id, "_") != 0 ){
                        generate_type(ID,temp->id,temp->depth);
                        code_insert(" GF@exp");
                        code_insert("\n");
                    }
                } else {
                    code_insert("TF@");
                    code_insert("$retval");
                    int_code_insert(sa_data.retval_counter);
                    int_code_insert(1); 
                    sa_data.retval_counter++;
                    code_insert(" GF@exp");
                    code_insert("\n");
                }
                free(temp);
            }
        } else free_and_exit(SEM_ERR);
        expression_list();
    }else if (cmp_type(END_LINE)){
        
    } else free_and_exit(SYNTAX_ERR);
}

// getting token and comparing its type
// returning true if the types are equal
bool get_token_cmp_type (T_types type){
    get_token();
    return (TOKEN_TYPE == type);
}

// comaring token type without getting another token
// returning true if the types are equal
bool cmp_type(T_types type)
{
    return (TOKEN_TYPE == type);
}

void skip_endline(){
    while (cmp_type(END_LINE)){
        get_token();
    }
}


//functions for list of types of left or right sides of equations (acting as stack)
void add_type_node(T_type_list** node, int type){
    
    T_type_list* tmp = malloc(sizeof(T_type_list));
    if(tmp == NULL)
        free_and_exit(INTERNAL_ERR);
    tmp->type = type;
    
    if((*node) == NULL){
        tmp->next = NULL;
        (*node) = tmp;
    } else {
        tmp->next = (*node);
        (*node) = tmp;
    }
}


void dispose_type_list(T_type_list** node){
    T_type_list* del;
    while((*node) != NULL){
        del = (*node);
        (*node) = (*node)->next;
        free(del);
    }
}

// comparing type list used for left and right sides of equations and parameters or return values
void cmp_multiple_types(T_type_list* list1,T_type_list* list2,bool params)
{
    T_type_list* temp, *temp1;
    temp = list1;
    temp1 = list2;
    while (temp != NULL && temp1 != NULL)
    {
        if (temp->type != temp1->type && (temp->type != UNDERSCORE))
        {
            if (params) {
                free_and_exit(PAR_ERR);
            } else {
                free_and_exit(TYPE_ERR);
            }
        }

        temp = temp->next;
        temp1 = temp1->next;
    }
    if (temp != NULL || temp1 != NULL)
    {
        if (params)
        {
            free_and_exit(PAR_ERR); 
        } else {
            free_and_exit(SEM_ERR);
        }
    }
}

// function for list used for ids on the left side of equation 
void add_id_node(T_left_side_id_list** node, char *ptr, int depth)
{  
    T_left_side_id_list* ins, *temp = *node, *prev = *node;
    ins = (T_left_side_id_list*) malloc(sizeof(struct left_side_id_list));
    if (ins == NULL) 
        free_and_exit(INTERNAL_ERR);
    ins->id = ptr;
    ins->next = NULL;
    ins->depth = depth;
    if (*node != NULL) {
        while(temp != NULL)
        {
            prev = temp;
            temp = temp->next;
        }
        prev->next = ins;
    } else {
        *node = ins;
    }

}

// getting the first value in a list
T_left_side_id_list* pop_id(T_left_side_id_list**node)
{
    T_left_side_id_list* del = *node;
    if (*node != NULL)
    {
        del = (*node);
        *node = (*node)->next;
    }
    return del;
}

void dispose_left_side_id(T_left_side_id_list** node)
{
    T_left_side_id_list* del;
    while((*node) != NULL){
        del = (*node);
        (*node) = (*node)->next;
        free(del);
    }
}

//geting the last value of a list (acting as stack)
T_left_side_id_list* pop_end_id(T_left_side_id_list**node){
    T_left_side_id_list* temp = *node;

    if (temp != NULL) {
        if (temp->next == NULL) { 
            *node = NULL;
            return temp; 
        } 
  
        T_left_side_id_list* del = *node; 
        while (del->next->next != NULL) 
        {
            del=del->next;
        }
        temp = del->next;
        del->next = NULL; 
    } 
    return temp;
}

//checking if all functions used were declared (inorder passage of functions binary tree)
// if not, the program exits with error code 3
void check_func_declaration(T_BTNode* ptr)
{
	if (ptr->declared == false)
	{
		free_and_exit(DEF_ERR);
	}
}