/////////////////////////////////////////////////////////
//                                                     //
// Implementace překladače imperativního jazyka IFJ20. //
//                   ~ code_gen.c ~                    //
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
//                           Code generation
//-------------------------------------------------------------------------
// Generation of IFJcode20 called during semantic analysis
// (using sprintf into char* in case of an error).
// There are two strings because of single pass syntax analysis.
// Function bodies are printed to different char* than main body, then if
// all succeeds functions are printed first.
// 


//code generation for string 
void code_insert(char* string_in){ 
    if(sa_data.inside_func == NULL || (strcmp(sa_data.inside_func->Key, "main") == 0)){
        if (sa_data.main_code.used+strlen(string_in) > sa_data.main_code.alocated)
        {
            sa_data.main_code.alocated += CODE_SIZE;
            char* temp = realloc(sa_data.main_code.output,sa_data.main_code.alocated);
            if (temp == NULL)
            {
                free_and_exit(INTERNAL_ERR); 
            } else {
                sa_data.main_code.output = temp;
            }
        }
        
        sa_data.main_code.used += sprintf(sa_data.main_code.output+sa_data.main_code.used, "%s", string_in);
    } else {
        if (sa_data.func_code.used+strlen(string_in) > sa_data.func_code.alocated)
        {
            sa_data.func_code.alocated += CODE_SIZE;
            char* temp = realloc(sa_data.func_code.output,sa_data.func_code.alocated);
            if (temp == NULL)
            {
                free_and_exit(INTERNAL_ERR); 
            } else {
                sa_data.func_code.output = temp;
            }
        }

        sa_data.func_code.used += sprintf(sa_data.func_code.output+sa_data.func_code.used, "%s", string_in);
    }
}

//code generation for integer
void int_code_insert(int int_in){ 
    size_t needed = snprintf(NULL, 0, "%d", int_in) + 1;
    if(sa_data.inside_func == NULL || (strcmp(sa_data.inside_func->Key, "main") == 0)){
        if (sa_data.main_code.used+needed  > sa_data.main_code.alocated)
        {
            sa_data.main_code.alocated += CODE_SIZE;
            char* temp = realloc(sa_data.main_code.output,sa_data.main_code.alocated);
            if (temp == NULL)
            {
                free_and_exit(INTERNAL_ERR); 
            } else {
                sa_data.main_code.output = temp;
            }
        }
        
        sa_data.main_code.used += sprintf(sa_data.main_code.output+sa_data.main_code.used, "%d", int_in);
    } else {
        if (sa_data.func_code.used+needed  > sa_data.func_code.alocated)
        {
            sa_data.func_code.alocated += CODE_SIZE;
            char* temp = realloc(sa_data.func_code.output,sa_data.func_code.alocated);
            if (temp == NULL)
            {
                free_and_exit(INTERNAL_ERR); 
            } else {
                sa_data.func_code.output = temp;
            }
        }

        sa_data.func_code.used += sprintf(sa_data.func_code.output+sa_data.func_code.used, "%d", int_in);
    }
}

//code generation for float
void float_code_insert(double float_in){ 
    size_t needed = snprintf(NULL, 0, "%a", float_in) + 1;
    if(sa_data.inside_func == NULL || (strcmp(sa_data.inside_func->Key, "main") == 0)){
        if (sa_data.main_code.used+needed  > sa_data.main_code.alocated)
        {
            sa_data.main_code.alocated += CODE_SIZE;
            char* temp = realloc(sa_data.main_code.output,sa_data.main_code.alocated);
            if (temp == NULL)
            {
                free_and_exit(INTERNAL_ERR); 
            } else {
                sa_data.main_code.output = temp;
            }
        }
        
        sa_data.main_code.used += sprintf(sa_data.main_code.output+sa_data.main_code.used, "%a", float_in);
    } else {
        if (sa_data.func_code.used+needed > sa_data.func_code.alocated)
        {
            sa_data.func_code.alocated += CODE_SIZE;
            char* temp = realloc(sa_data.func_code.output,sa_data.func_code.alocated);
            if (temp == NULL)
            {
                free_and_exit(INTERNAL_ERR); 
            } else {
                sa_data.func_code.output = temp;
            }
        }

        sa_data.func_code.used += sprintf(sa_data.func_code.output+sa_data.func_code.used, "%a", float_in);
    }
}

//generation of variables and constants in IFJcode20
void generate_type(T_types type, char* value, int depth){ 

    if(isdigit(value[0]) || type == QM_STRING ) {
        switch(type){
            case INT:
                code_insert("int@");
                code_insert(value);
                break;
            case FLOAT:
                code_insert("float@");
                double tmp = atof(value);
                float_code_insert(tmp);
                break;
            case QM_STRING:
                code_insert("string@");
                code_insert(value);
                break;
            default:
                free_and_exit(TYPE_ERR);    
                break;
        }
    } else {
        code_insert("TF@");
        code_insert(value);
        int_code_insert(depth);
    }
}


void code_gen_clear_frame(){
    T_SA_stack* temp = sa_stack;
    while(temp != sa_data.func_symtab){
      
        BTInorder(temp->local_symtab,code_gen_keep_variables);
        temp = temp->next;
    }
    T_left_side_id_list* ids;
    code_insert("CREATEFRAME\n");
    while(sa_data.left_side_ids != NULL){
        ids = pop_end_id(&(sa_data.left_side_ids));
        code_insert("DEFVAR TF@");
        code_insert(ids->id);
        int_code_insert(ids->depth);
        code_insert("\n");
        code_insert("POPS TF@");
        code_insert(ids->id);
        int_code_insert(ids->depth);
        code_insert("\n");
        free(ids);
    }
    
}

//pushing variables to stack before cleaning Temporary frame
void code_gen_keep_variables(T_BTNode* ptr){ 
    if (ptr->type != UNDERSCORE)
    {
        code_insert("PUSHS TF@");
        code_insert(ptr->Key);
        add_id_node(&(sa_data.left_side_ids), ptr->Key, ptr->depth);
        int_code_insert(ptr->depth);
        code_insert("\n");
    }
}

//generating code depending on the operation used by user
void code_gen_operation()
{
    switch(PA_STACK(1).type){
        case PLUS:
            if (PA_STACK(0).type == QM_STRING)
            {
               code_insert("POPS GF@exp\n POPS GF@exp1\n");
               code_insert("CONCAT GF@exp GF@exp1 GF@exp\n");
               code_insert("PUSHS GF@exp\n");
            } else { 
                code_insert("ADDS\n");
            }
            break;
        case MINUS:
              code_insert("SUBS\n");
            if (PA_STACK(0).type == QM_STRING)
                free_and_exit(TYPE_ERR);
            break;
        case MULTIPLICATION:
            code_insert("MULS\n");
            if (PA_STACK(0).type == QM_STRING)
                free_and_exit(TYPE_ERR);
            break;
        case DIVISION:
            
            if (PA_STACK(0).type == FLOAT) {
                code_insert("JUMPIFEQ divzero_err float@");
                float_code_insert(0.0);
                code_insert(" ");
                generate_type(PA_STACK(0).type, PA_STACK(0).value, PA_STACK(0).depth);
                code_insert("\n");
                code_insert("DIVS\n");
            } else{ 
                code_insert("JUMPIFEQ divzero_err int@0 ");
                generate_type(PA_STACK(0).type, PA_STACK(0).value, PA_STACK(0).depth);
                code_insert("\n");
                code_insert("IDIVS\n");
            }
            if (PA_STACK(0).type == QM_STRING)                 
                free_and_exit(TYPE_ERR);

            if (PA_STACK(0).is_zero)        
                free_and_exit(ZERO_DIVISON);
            break;
        case LESS_THAN:
            if(sa_data.cmp_used)
                free_and_exit(TYPE_ERR);
           code_insert("LTS\n");
            sa_data.cmp_used = true;
                break;
        case MORE_THAN:
            if(sa_data.cmp_used)
                free_and_exit(TYPE_ERR);
            code_insert("GTS\n");
            sa_data.cmp_used = true;
            break;
        case LESS_THAN_EQ:
            if(sa_data.cmp_used)
                free_and_exit(TYPE_ERR);
            code_insert("GTS\n");
            sa_data.cmp_used = true;
            break;
        case MORE_THAN_EQ:
            if(sa_data.cmp_used)
                free_and_exit(TYPE_ERR);
            code_insert("LTS\n");
            sa_data.cmp_used = true;
            break;                 
        case CMP_EQUAL:
            if(sa_data.cmp_used)
                free_and_exit(TYPE_ERR);
            code_insert("EQS\n");
            sa_data.cmp_used = true;
            break;
        case NOT_EQUAL:
            if(sa_data.cmp_used)
                free_and_exit(TYPE_ERR);
           code_insert("EQS\n");
            sa_data.cmp_used = true;
            break;
        default:
            break;
    }
    if(PA_STACK(1).type == LESS_THAN_EQ || PA_STACK(1).type == MORE_THAN_EQ || PA_STACK(1).type == NOT_EQUAL){  
        code_insert("NOTS\n");
    }
} 


//generation of builtin functions
void code_gen_builtin(){
    code_insert("\n");
    code_inputi();
    code_insert("\n");
    code_inputs();
    code_insert("\n");
    code_inputf();
    code_insert("\n");
    code_int2float();
    code_insert("\n");
    code_float2int();
    code_insert("\n");
    code_len();
    code_insert("\n");
    code_substr();
    code_insert("\n");
    code_ord();
    code_insert("\n");
    code_chr();
    code_insert("\n");
}
// individual builtin code generation 
void code_inputi(){


    code_insert("LABEL inputi\n");                        
    code_insert("CREATEFRAME\n");                         
    code_insert("DEFVAR TF@$retval01\n");                 
    code_insert("DEFVAR TF@$retval11\n");   
    code_insert("MOVE TF@$retval11 int@0\n");             
    code_insert("DEFVAR TF@lval\n");                      
    code_insert("READ TF@$retval01 int\n");               
    code_insert("EQ TF@lval TF@$retval01 nil@nil\n");     
    code_insert("JUMPIFEQ badinputi TF@lval bool@true\n");
    code_insert("JUMP endinputi\n");                      
    code_insert("LABEL badinputi\n");                     
    code_insert("MOVE TF@$retval11 int@1\n");             
    code_insert("MOVE TF@$retval01 int@0\n");
    code_insert("LABEL endinputi\n");                     
    code_insert("RETURN\n");                              

}

void code_inputs(){

    code_insert("LABEL inputs\n");                        
    code_insert("CREATEFRAME\n");                         
    code_insert("DEFVAR TF@$retval01\n");                 
    code_insert("DEFVAR TF@$retval11\n"); 
    code_insert("MOVE TF@$retval01 int@0\n"); 
    code_insert("MOVE TF@$retval11 int@0\n");             
    code_insert("DEFVAR TF@lval\n");                      
    code_insert("MOVE TF@$retval11 int@0\n");             
    code_insert("READ TF@$retval01 string\n");            
    code_insert("EQ TF@lval TF@$retval01 nil@nil\n");     
    code_insert("JUMPIFEQ badinputs TF@lval bool@true\n");
    code_insert("JUMP endinputs\n");                      
    code_insert("LABEL badinputs\n");                     
    code_insert("MOVE TF@$retval11 int@1\n");             
    code_insert("LABEL endinputs\n");                     
    code_insert("RETURN\n");                              

}

void code_inputf(){

    code_insert("LABEL inputf\n");                        
    code_insert("CREATEFRAME\n");                         
    code_insert("DEFVAR TF@$retval01\n");                 
    code_insert("DEFVAR TF@$retval11\n");                 
    code_insert("MOVE TF@$retval01 int@0\n"); 
    code_insert("MOVE TF@$retval11 int@0\n");
    code_insert("DEFVAR TF@lval\n");                      
    code_insert("MOVE TF@$retval11 int@0\n");             
    code_insert("READ TF@$retval01 float\n");             
    code_insert("EQ TF@lval TF@$retval01 nil@nil\n");     
    code_insert("JUMPIFEQ badinputf TF@lval bool@true\n");
    code_insert("JUMP endinputf\n");                      
    code_insert("LABEL badinputf\n");                     
    code_insert("MOVE TF@$retval11 int@1\n");             
    code_insert("LABEL endinputf\n");                     
    code_insert("RETURN\n");                              
}

void code_int2float(){

    code_insert("LABEL int2float\n");                     
    code_insert("CREATEFRAME\n");                         
    code_insert("DEFVAR TF@$retval01\n");                 
    code_insert("MOVE TF@$retval01 int@0\n"); 
    code_insert("DEFVAR TF@i\n");                         
    code_insert("POPS TF@i\n");                           
    code_insert("INT2FLOAT TF@$retval01 TF@i\n");         
    code_insert("RETURN\n");                              
} 

void code_float2int(){

    code_insert("LABEL float2int\n");                     
    code_insert("CREATEFRAME\n");                         
    code_insert("DEFVAR TF@f\n");                         
    code_insert("DEFVAR TF@$retval01\n");                 
    code_insert("MOVE TF@$retval01 int@0\n"); 
    code_insert("POPS TF@f\n");                    
    code_insert("FLOAT2INT TF@$retval01 TF@f\n");  
    code_insert("RETURN\n");                       
  
}

void code_len(){

    code_insert("LABEL len\n");                    
    code_insert("CREATEFRAME\n");                  
    code_insert("DEFVAR TF@s\n");                  
    code_insert("DEFVAR TF@$retval01\n");          
    code_insert("MOVE TF@$retval01 int@0\n"); 
    code_insert("POPS TF@s\n");                    
    code_insert("STRLEN TF@$retval01 TF@s\n");     
    code_insert("RETURN\n");                       
    
}
void code_substr(){

    code_insert("LABEL substr\n");                 
    code_insert("CREATEFRAME\n");                  
    code_insert("DEFVAR TF@s\n");                  
    code_insert("DEFVAR TF@i\n");                  
    code_insert("DEFVAR TF@n\n");                  
    code_insert("DEFVAR TF@$retval01\n");          
    code_insert("MOVE TF@$retval01 int@0\n"); 
    code_insert("DEFVAR TF@strtmp\n");             
    code_insert("DEFVAR TF@lenret\n");             
    code_insert("DEFVAR TF@$retval11\n");          
    code_insert("MOVE TF@$retval11 int@0\n"); 
    code_insert("DEFVAR TF@tval\n");               
    code_insert("DEFVAR TF@counter\n");            
    code_insert("POPS TF@n\n");                    
    code_insert("POPS TF@i\n");                    
    code_insert("POPS TF@s\n");                    
    code_insert("PUSHS TF@s\n");                   
    code_insert("MOVE TF@counter int@0\n");        
    code_insert("MOVE TF@$retval01 string@\n");    
    code_insert("MOVE TF@$retval11 int@0\n");      
    code_insert("PUSHFRAME\n");                                 
    code_insert("CALL len\n");                                  
    code_insert("MOVE LF@lenret TF@$retval01\n");               
    code_insert("POPFRAME\n");                                  
    code_insert("SUB TF@lenret TF@lenret int@1\n");             
    code_insert("LT TF@tval TF@i int@0\n");                     
    code_insert("JUMPIFEQ badsubstr TF@tval bool@true\n");      
    code_insert("LT TF@tval TF@lenret TF@i\n");                 
    code_insert("JUMPIFEQ badsubstr TF@tval bool@true\n");      
    code_insert("LT TF@tval TF@n int@0\n");                     
    code_insert("JUMPIFEQ badsubstr TF@tval bool@true\n");      
    code_insert("EQ TF@tval TF@n int@0\n");                     
    code_insert("JUMPIFEQ nilsubstr TF@tval bool@true\n");      
    code_insert("ADD TF@n TF@n TF@i\n");                        
    code_insert("ADD TF@lenret TF@lenret int@1\n");             
    code_insert("LT TF@tval TF@n TF@lenret\n");                 
    code_insert("JUMPIFEQ newnesubstr TF@tval bool@true\n");    
    code_insert("MOVE TF@n TF@lenret\n");                       
    code_insert("JUMPIFEQ newsubstr TF@n TF@i\n");              
    code_insert("LABEL newnesubstr\n");                         
    code_insert("MOVE TF@counter TF@i\n");                      
    code_insert("GETCHAR TF@$retval01 TF@s TF@counter\n");      
    code_insert("ADD TF@counter TF@counter int@1\n");           
    code_insert("JUMPIFEQ endsubstr TF@counter TF@n\n");        
    code_insert("LABEL loopsubstr\n");                          
    code_insert("GETCHAR TF@strtmp TF@s TF@counter\n");         
    code_insert("CONCAT TF@$retval01 TF@$retval01 TF@strtmp\n");
    code_insert("ADD TF@counter TF@counter int@1\n");           
    code_insert("JUMPIFNEQ loopsubstr TF@counter TF@n\n");      
    code_insert("JUMP endsubstr\n");                            
    code_insert("LABEL badsubstr\n");                           
    code_insert("MOVE TF@$retval11 int@1\n");                   
    code_insert("LABEL nilsubstr\n");                           
    code_insert("MOVE TF@$retval01 string@\n");     
    code_insert("LABEL endsubstr\n");               
    code_insert("RETURN\n");                        

}

void code_ord(){

    code_insert("LABEL ord\n");                     
    code_insert("CREATEFRAME\n");                   
    code_insert("DEFVAR TF@s\n");                   
    code_insert("DEFVAR TF@i\n");                   
    code_insert("DEFVAR TF@$retval01\n");           
    code_insert("DEFVAR TF@$retval11\n");           
    code_insert("MOVE TF@$retval01 int@0\n"); 
    code_insert("MOVE TF@$retval11 int@0\n"); 
    code_insert("DEFVAR TF@lval\n");                
    code_insert("DEFVAR TF@lenret\n");              
    code_insert("DEFVAR TF@char\n");                
    code_insert("POPS TF@i\n");                     
    code_insert("POPS TF@s\n");                     
    code_insert("MOVE TF@$retval11 int@0\n");       
    code_insert("PUSHS TF@s\n");                   
    code_insert("PUSHFRAME\n");                                 
    code_insert("CALL len\n");                                  
    code_insert("MOVE LF@lenret TF@$retval01\n");             
    code_insert("POPFRAME\n");                                  
    code_insert("LT TF@lval TF@i int@0\n");                     
    code_insert("JUMPIFEQ badord TF@lval bool@true\n");        
    code_insert("SUB TF@lenret TF@lenret int@1\n");           
    code_insert("LT TF@lval TF@lenret TF@i\n");                 
    code_insert("JUMPIFEQ badord TF@lval bool@true\n");     
    code_insert("GETCHAR TF@char TF@s TF@i\n");                 
    code_insert("STRI2INT TF@$retval01 TF@char int@0\n");       
    code_insert("JUMP endord\n");                               
    code_insert("LABEL badord\n");                              
    code_insert("MOVE TF@$retval11 int@1\n");                   
    code_insert("MOVE TF@$retval01 int@0\n");                   
    code_insert("LABEL endord\n");                              
    code_insert("RETURN\n");                                    
}

void code_chr(){

    code_insert("LABEL chr\n");                                 
    code_insert("CREATEFRAME\n");                               
    code_insert("DEFVAR TF@i\n");                               
    code_insert("DEFVAR TF@$retval01\n");                       
    code_insert("DEFVAR TF@$retval11\n");                       
    code_insert("MOVE TF@$retval01 int@0\n"); 
    code_insert("MOVE TF@$retval11 int@0\n"); 
    code_insert("DEFVAR TF@lval\n");                            
    code_insert("MOVE TF@$retval11 int@0\n");                   
    code_insert("POPS TF@i\n");                     
    code_insert("LT TF@lval TF@i int@0\n");             
    code_insert("JUMPIFEQ badchr TF@lval bool@true\n");  
    code_insert("LT TF@lval int@255 TF@i\n");            
    code_insert("JUMPIFEQ badchr TF@lval bool@true\n");  
    code_insert("INT2CHAR TF@$retval01 TF@i\n");
    code_insert("JUMP endchr\n");
    code_insert("LABEL badchr\n");
    code_insert("MOVE TF@$retval11 int@1\n");
    code_insert("MOVE TF@$retval01 string@0");
    code_insert("\n");
    code_insert("LABEL endchr\n");
    code_insert("RETURN\n");
}