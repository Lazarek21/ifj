/////////////////////////////////////////////////////////
//                                                     //
// Implementace překladače imperativního jazyka IFJ20. //
//                     ~ LA.h ~                        //
//                                                     //
//              Hynek Šabacký (xsabac02),              //
//             Mikuláš Brázda (xbrazd21),              //
//              David Lorenc (xloren16)                //
//                                                     //
//               Last edit: 5. 12. 2020                //
///////////////////////////////////////////////////////// 

#include "main.h"

#define CHARS 128
#define SIZE 8
#define CHAR(idx) (*c)[(idx)] 


enum qmstates {INIT = 0,STRING = 1, ESCAPE = 2, HEX1 = 3, HEX2 = 4, END = 5};






typedef int FUNC(char**, int*);
// array of pointers to functions
// each function operates on character which is placed on same index 
// in ASCII table as function in f_ptrs table
                            //0        1        2       3          4     
FUNC *f_ptrs[CHARS] = {     NULL,    NULL,    NULL,    NULL,    NULL,       //0
                             NULL,    NULL,    NULL,    NULL,whitespace,    //5
                             end_line,NULL,    NULL,    end_line,NULL,      //10
                             NULL,    NULL,    NULL,    NULL,    NULL,      //15
                             NULL,    NULL,    NULL,    NULL,    NULL,      //20
                             NULL,    NULL,    NULL,    NULL,    NULL,      //25
                             NULL,    NULL,whitespace,not_equal,qm_string,  //30
                             NULL,    NULL,    NULL,    NULL,    NULL,      //35
                             l_parenth,r_parenth,multiplication,plus,comma, //40
                             minus,   NULL, f_slash,    zero,    int_proc,  //45
                             int_proc,int_proc,int_proc,int_proc,int_proc,  //50
                             int_proc,int_proc,int_proc,colon,semicolon,    //55
                             less_than,equal,more_than,  NULL,   NULL,      //60
                             id_proc,id_proc,id_proc,id_proc,id_proc,       //65
                             id_proc,id_proc,id_proc,id_proc,id_proc,       //70
                             id_proc,id_proc,id_proc,id_proc,id_proc,       //75
                             id_proc,id_proc,id_proc,id_proc,id_proc,       //80
                             id_proc,id_proc,id_proc,id_proc,id_proc,       //85
                             id_proc, NULL,    NULL,    NULL,    NULL,      //90
                             underscore, NULL, id_proc,id_proc,id_proc,     //95
                             id_proc,id_proc,id_proc,id_proc,id_proc,       //100
                             id_proc,id_proc,id_proc,id_proc,id_proc,       //105
                             id_proc,id_proc,id_proc,id_proc,id_proc,       //110
                             id_proc,id_proc,id_proc,id_proc,id_proc,       //115
                             id_proc, id_proc,id_proc,start_block, NULL,    //120
                             end_block, NULL, NULL};                        //125
