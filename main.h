/////////////////////////////////////////////////////////
//                                                     //
// Implementace překladače imperativního jazyka IFJ20. //
//                    ~ main.h ~                       //
//                                                     //
//              Hynek Šabacký (xsabac02),              //
//             Mikuláš Brázda (xbrazd21),              //
//              David Lorenc (xloren16)                //
//                                                     //
//               Last edit: 5. 12. 2020                //
///////////////////////////////////////////////////////// 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#define SIZE 8                          //default size of incomming token 
#define MAXSTACK 64                     //max size of helper stack for symtab
#define PA_MAXSTACK 32                  //max size of helper stack for bottom-up analysis
#define CODE_SIZE 4096                  //default size of output code

// enumeration of all different possible blocks of code
typedef enum blocks  { B_IF = 0,B_FOR = 1, B_ELSE = 2} T_blocks;

//enumeration of all different possible exit codes
enum retvals {ALL_PASS = 0,LA_EOF = -1, LA_ERR = 1, SYNTAX_ERR = 2,DEF_ERR = 3, TYPE_ERR = 5, PAR_ERR = 6,SEM_ERR =7,ZERO_DIVISON = 9,INTERNAL_ERR = 99};

// enumeration of all the different token types accepted by lexical analysis
typedef enum types {UNDEFINED =-1, PLUS = 0, MINUS = 1,MULTIPLICATION = 2, DIVISION = 3, L_PARENTH = 4, 
            R_PARENTH = 5, ID = 6, INT = 7,FLOAT = 8, QM_STRING = 9,
            LESS_THAN = 10, MORE_THAN = 11, LESS_THAN_EQ = 12,MORE_THAN_EQ = 13, CMP_EQUAL = 14, NOT_EQUAL = 15,PA_ENDCHAR = 16, // PA_ENDCHAR is used in
            SEMICOLON = 17, DECLARATION = 18, END_LINE = 19,UNDERSCORE = 20, START_BLOCK = 21, END_BLOCK = 22,                   // expression as an end of input.
            KW_FUNC = 23, KW_FLOAT64 = 24,KW_ELSE = 25, KW_FOR = 26, KW_IF = 27, KW_INT =28, KW_PACKAGE = 29, 
            KW_RETURN = 30, KW_STRING = 31,KW_MAIN = 32, EOIF = 33, COMMA = 34, EQUAL = 35} T_types;


// list acting as stack of data types used for left side of equation
typedef struct left_side_types {
    T_types type;                       //item type
    struct left_side_types* next;       //pointer to the next item
} T_type_list;

// list of variable names used for left side of equation
typedef struct left_side_id_list {
    char* id;                           //id name
    int depth;                          //id immersion
    struct left_side_id_list* next;     //pointer to the next item
} T_left_side_id_list;

// item in symtable (implemented as a binary tree)
typedef struct BTNode	{                                     
    char* Key;                          //name of the variable 
    int type;                           //int,float64,qm_string
    int depth;                          //automatically filled with sa_stack->item_index
    bool declared;                      //whether the item was declared
    bool used;                          //whether the item was declared         
    T_type_list* args;                  //list acting as stack of parameter types
    T_type_list* rets;                  //list acting as stack of return value types        
    struct BTNode * LPtr;               //pointer to left child                              
    struct BTNode * RPtr;               //pointer to right child  	  	                            
} T_BTNode;        

//item in a stack used for expression evaluation
typedef struct PA_data {
    bool terminal;                      //whether the item is terminal
    bool handle;                        //whether the item is handle
    int  type;                          //data type of the item
    bool is_zero;                       //whether the item is zero
    char* value;                        //name of id / value of constant
    bool is_constant;                   //whether the item is constant or variable
    int depth;                          //items depth of immersion
} T_PA_data;

//stack used for expression
typedef struct PA_stack {              
    int top;                            //number of items in stack
    T_PA_data arr[PA_MAXSTACK];         //array of items
} T_PA_stack;

//stack used in symtable for iterative passage
typedef struct	{                          
    T_BTNode* a[MAXSTACK];              //array of tree nodes
    int top;                            //number of items
} tStackP;

 //stack with symtables, last one is function symtab
typedef struct SA_stack { 
    int item_index;                     //depth of immersion
    int label_index;                    //unique key for labels
    T_blocks type;                      //block type (0 - if, 1 - for, 2 - else)
    T_BTNode* local_symtab;             //pointer to a local symtab
    struct SA_stack * next;             //pointer to next stack items
} T_SA_stack;

//token passed to syntax analysis from lexical analysis
typedef struct Token {
    int type;                           //type of token
    char *data;                         //value / name / char
} T_token;

//string used for code generation
typedef struct code {
    int used;                           //number of bytes used
    int alocated;                       //number of bytes alocated
    char* output;                       //string of code to be generated if no errors found
} T_code;

//global data used in most of functions
typedef struct SA_data {
    bool main_found;                    //whether main was found in code
    bool return_found;                  //whether return was found in function
    bool inside_return;                 //whether the program currently checking the correctness of return
    T_token* token;                     //token passed from scanner
    T_BTNode* func_current_node;        //last used function
    T_BTNode* inside_func;              //inside of which function delcaration
    T_BTNode* var_current_node;         //last used variable
    T_SA_stack* func_symtab;            //function symtable
    T_BTNode* previous_var_node;        //help variable to keep wanted tree node
    T_type_list* left_side_types;       //list (stack) of data types used for left side of equation
    T_type_list* right_side_types;      //list (stack) of data types used for right side of equation
    T_code main_code;                   //string into which main code is generated
    T_code func_code;                   //string into which function code is generated
    bool cmp_used;                      //true - if relational operator was used in expression 
    int retval_counter;                 //unique key for helping return variables
    unsigned int label_counter;         //unique key for labels
    T_left_side_id_list* left_side_ids; //list of variable names used for left side of equation
    T_PA_stack S;                       //stack used in expression
} T_SA_data;


/*********** MAIN.C *********/ 
void prologue();
void declaration_list();
void param_list();
void param();
int  type();
void return_list();
void return_vals();
void statement_list();
void statement();
void statement_id();
void leftside_id();
void Init_func_symtab(T_BTNode** rootPtr);
void arg_list();
void arg();
void value();
void expression_list();
void skip_endline();
void InitSa_data();
void get_token();
void free_and_exit(int exit_code);
void free_resources();
bool cmp_type(T_types type);
bool get_token_cmp_type (T_types type);
void add_type_node(T_type_list** node, int type);
void dispose_type_list(T_type_list** node);
void cmp_multiple_types(T_type_list* list1,T_type_list* list2,bool params);


void add_id_node(T_left_side_id_list** node, char*ptr, int depth);
void dispose_left_side_id(T_left_side_id_list** node);
T_left_side_id_list* pop_id(T_left_side_id_list**node);
T_left_side_id_list* pop_end_id(T_left_side_id_list**node);

/*********** SYMTAB.C *********/ 
void S_push(tStackP *S, T_BTNode* ptr);
void S_init(tStackP *S);
T_BTNode* S_top_pop(tStackP *S);
bool S_empty (tStackP *S);
T_BTNode* BT_insert(T_BTNode** RootPtr, const char* Key);
void BT_dispose_tree(T_BTNode** RootPtr);
T_BTNode* BT_search(T_SA_stack* start, T_SA_stack* end, const char* Key);
T_BTNode* BT_local_search(T_BTNode* RootPtr, const char* Key);
T_BTNode* BT_search_and_insert(T_SA_stack* start, T_SA_stack* end, const char* Key, T_BTNode** dest);
void check_func_declaration (T_BTNode* RootPtr);
void Leftmost_Inorder(T_BTNode* ptr, tStackP *Stack);
void BTInorder (T_BTNode* RootPtr,void (*operation)());

/*********** LA.C *********/ 
int scanner(int *token, char** data);
int end_line(char** c,  int* type);
int whitespace(char** c,int *type);
int not_equal(char** c, int* type);
int qm_string(char** c, int* type);
int l_parenth(char** c, int* type);
int r_parenth(char** c, int* type);
int multiplication(char** c, int* type);
int plus(char** c, int* type);
int comma(char** c, int* type);
int minus(char** c, int* type);
int f_slash(char** c, int* type);
int zero(char** c, int* type);
int float_dot(char** c, int* type);
int float_e(char** c, int* type);
int int_proc(char** c, int* type);
int colon(char** c, int* type);
int semicolon(char** c, int* type);
int less_than(char** c, int* type);
int equal(char** c, int* type);
int more_than(char** c, int* type);
int id_proc(char** c, int* type);
int underscore(char** c, int* type);
int start_block(char** c, int* type);
int end_block(char** c, int* type);
void getchar_check_mem(char** c, int* i);
void check_mem(char**c,int* i);

/***********EXPRESSION.C**********/
int expression(bool cmp_disabled, int dollar_transform);
void SPA_dispose();
void SPA_init ();
void SPA_push (T_PA_data data, char* value);
void SPA_pop ();
int SPA_first_terminal();
bool SPA_empty ();
void SPA_reduce_stack(bool* cmp_disabled);

/*********** SA_STACK.C *********/ 
void sa_stack_init(T_SA_stack** SAStack);
void sa_stack_push(T_SA_stack** SAStack, T_blocks temp_type);
void sa_stack_pop(T_SA_stack** SAStack);
void sa_stack_empty(T_SA_stack** SAStack);
T_BTNode* sa_stack_search(T_SA_stack** SAStack, const char* Key);

/*********** CODE_GEN.C *********/
void code_insert(char* string_in);
void int_code_insert(int int_in);
void generate_type(T_types type, char* value, int depth);
void float_code_insert(double float_in);
void code_gen_builtin();
void code_inputi();
void code_inputs();
void code_inputf();
void code_int2float();
void code_float2int();
void code_len();
void code_substr();
void code_ord();
void code_chr();
void code_gen_clear_frame();
void code_gen_keep_variables(T_BTNode* ptr);
void code_gen_operation();


/*********HELPER MACROS***********/
#define LOCAL_SYMTAB (&(sa_stack->local_symtab))
#define FUNC_SYMTAB (&(sa_data.func_symtab->local_symtab))
#define TOKEN_DATA (sa_data.token->data)
#define TOKEN_TYPE (sa_data.token->type)
#define FUNC_CURRENT (sa_data.func_current_node)
#define VAR_CURRENT (sa_data.var_current_node)
#define PA_STACK(idx) (sa_data.S.arr[sa_data.S.top-idx-1])