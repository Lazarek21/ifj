/////////////////////////////////////////////////////////
//                                                     //
// Implementace překladače imperativního jazyka IFJ20. //
//                     ~ LA.c ~                        //
//                                                     //
//              Hynek Šabacký (xsabac02),              //
//             Mikuláš Brázda (xbrazd21),              //
//              David Lorenc (xloren16)                //
//                                                     //
//               Last edit: 5. 12. 2020                //
///////////////////////////////////////////////////////// 

#include "LA.h" 

//-------------------------------------------------------------------------
//                           Scanner function
//-------------------------------------------------------------------------
// provides token to syntactic analysis
// based on fact, that each character is number represented by its 
// order in ASCII table
int scanner(int *type, char**data)
{
    int retval = ALL_PASS;
    static char p_c = 0;
    int size_out = 0;
    const char* keywords[10] = {"func","float64","else","for","if", "int","package","return", "string", "main"};
    for(int i=0; i<SIZE+1; i++)
        (*data)[i] = '\0';

    //p_c is 0 when 1st call of this function 
    (*data)[0] = p_c; 
    if (p_c == 0)
    {
        (*data)[0] = getchar();
    }
    
    if ((*data)[0] >= -1) 
    { 
        if((*data)[0] != EOF)
        {
            if (f_ptrs[(unsigned)((*data)[0])] != NULL)
            {
                retval = f_ptrs[(unsigned)((*data)[0])](data, type);
                size_out = strlen(*data)-1;
                p_c = (*data)[size_out];
                (*data)[size_out] = '\0';
            } else {
                retval = LA_ERR;
                p_c = getchar();
            }
        } else {
            *type = EOIF;
        }
    } else {
        retval = LA_ERR;
    }

    if (*type != QM_STRING)
    {
        for(int i=0; i<10; i++)
        {
            if(strcmp(*data, keywords[i]) == 0){
                *type = i+23;
            }
        }
    }
    
    return retval;
}

int whitespace(char** c,int *type)  
{
    int retval = ALL_PASS;
    while((CHAR(0) = getchar()) == ' ' || CHAR(0) == '\t')
    {}

    if(CHAR(0) != EOF)
    {
        if (f_ptrs[(unsigned)(CHAR(0))] != NULL)
        {
            retval = f_ptrs[(unsigned)(CHAR(0))](c,type);
        } else {
            retval = LA_ERR;
            CHAR(0) = getchar();
        }
    } else {
        retval = LA_EOF;
    }
    return retval;
}

int not_equal(char** c, int* type) 
{
    *type = NOT_EQUAL;
    int retval = ALL_PASS;
    CHAR(1)=getchar();
    if (CHAR(1) == '=')
    {
        CHAR(2) = getchar();        
    } else {
        retval = LA_ERR;
    }

    return retval;
}

int qm_string(char** c, int* type)  
{
    *type = QM_STRING;
    int i = -1;
    int retval = ALL_PASS;
    unsigned short state = STRING;
    int to_print = 0;

    while (state != END)
    {
        getchar_check_mem(c, &i);
        if (CHAR(i) == EOF)
            return LA_ERR;
        switch (state)
        {
            case STRING:
                switch (CHAR(i))
                {
                case '\\':
                    state = ESCAPE;
                    break;
                case '"':
                    CHAR(i) = '\0';
                    i--;
                    state = END;
                    break;
                case ' ':
                    CHAR(i) = '\\';
                    check_mem(c, &i);
                    check_mem(c, &i);
                    check_mem(c, &i);
                    CHAR(i-2) = '0';
                    CHAR(i-1) = '3';
                    CHAR(i) = '2';
                    break;

                
                default:
                    if(CHAR(i) < 32)
                    {
                        retval = LA_ERR;
                    } 
                    break;
                }
                break;
            case ESCAPE:
                check_mem(c,&i);
                check_mem(c,&i);
                switch (CHAR(i-2))
                {
                    case  '"' :
                        (*c)[i-2] = '0';
                        (*c)[i-1] = '3';
                        (*c)[i] = '4';
                        state = STRING;
                        break;
                    case  't' :
                        (*c)[i-2] = '0';
                        (*c)[i-1] = '0';
                        (*c)[i] =   '9';
                        state = STRING;
                        break;
                    case  '\\':
                        (*c)[i-2] = '0';
                        (*c)[i-1] = '9';
                        (*c)[i]   = '2';
                        state = STRING;
                        break;
                    case  'n':
                        (*c)[i-2] = '0';
                        (*c)[i-1] = '1';
                        (*c)[i]   = '0';
                        state = STRING;
                        break;
                    case 'x':
                        CHAR(i) = '0';
                        i--;
                        state = HEX1;
                        break;
                    default:
                        state = STRING;
                        retval = LA_ERR;
                        break;
                }
                to_print = 0;
                break;
            case HEX1:
                CHAR(i) = tolower(CHAR(i));
                if(!((CHAR(i) >= 'a' && CHAR(i) <= 'f')  || isdigit(CHAR(i))))
                {
                    if(CHAR(i) == '"')
                    {
                        state = END;
                    } else {
                        state = STRING;
                    }
                    retval = LA_ERR;
                } else {
                    state = HEX2;
                }
                if (isalpha(CHAR(i)))
                {
                    to_print += (CHAR(i)-87)*16;
                } else {
                    to_print += (CHAR(i)-48)*16;
                }
                break;
            case HEX2:
                CHAR(i) = tolower(CHAR(i));
                if(!((CHAR(i) >= 'a' && CHAR(i) <= 'f') || isdigit(CHAR(i))))
                {
                    if(CHAR(i) == '"')
                    {
                        state = END;
                    } 
                    retval = LA_ERR;
                }
                if (isalpha(CHAR(i)))
                {
                    to_print += (CHAR(i)-87);
                } else {
                    to_print += (CHAR(i)-48);
                }
                size_t needed = snprintf(NULL, 0, "%d", to_print);
                CHAR(i-1) = '0';
                CHAR(i-2) = '0';
                CHAR(i-3) = '0';
                sprintf((*c)+i-needed,"%d",to_print);
                i--;
                state = STRING;
                to_print = 0;

                break;
            default:
                break;
        }
    }
    getchar_check_mem(c, &i);
    return retval;
}

int l_parenth(char** c, int* type) 
{
    *type = L_PARENTH;
    CHAR(1) = getchar();
    return 0;
}

int r_parenth(char** c, int* type) 
{
    *type = R_PARENTH;
    CHAR(1) = getchar();
    return 0;
}

int multiplication(char** c, int* type) 
{
    *type = MULTIPLICATION;
    CHAR(1) = getchar();
    return 0;
}

int plus(char** c, int* type) 
{
    *type = PLUS;
    CHAR(1) = getchar();
    return 0;
}

int comma(char** c, int* type) 
{
    *type = COMMA;
    CHAR(1) = getchar();
    return 0;
}

int minus(char** c, int* type) 
{
    *type = MINUS;
    CHAR(1) = getchar();
    return 0;
}

int f_slash(char** c, int* type)  
{
    *type = DIVISION;
    bool star = false;
    int retval = ALL_PASS;
    CHAR(1) = getchar();
    switch (CHAR(1))
    {
    case '/':
        while ((CHAR(1) = getchar()) != '\n')
        {
            if(CHAR(1) == EOF)
            {
                return LA_EOF;
            }
        }
        CHAR(0)='\n';
        CHAR(1)='\0';
        retval = end_line(c,type);
        break;

    case '*':
        while (!((CHAR(1) = getchar()) == '/' && star == true))
        {
            if(CHAR(1) == '*') 
            {
                star = true;
            }else if(CHAR(1) == EOF){
                return LA_ERR;  
            } else {
                star = false;
            }
        }
        CHAR(0) = getchar();
        CHAR(1) = '\0';
        if(CHAR(0) == EOF) 
        {
            return LA_ERR;
        } else if (f_ptrs[(unsigned)(CHAR(0))] != NULL) {
            CHAR(1)='\0';
            retval = f_ptrs[(unsigned)(CHAR(0))](c,type);
        } else {
            CHAR(0) = getchar();
            CHAR(1) = '\0';
            retval = LA_ERR;
        }
        break;
    default: 
        break;
    }
 
    return retval;
}

int zero(char** c, int* type)  
{
    int retval = ALL_PASS;
    *type = INT;
    CHAR(1) = getchar();
    if(CHAR(1) == '.')
    {
        retval = float_dot(c,type);
    }
    else if(CHAR(1) == 'e' || CHAR(1) == 'E')
    {
        retval = float_e(c,type);
    }
    else if (isdigit(CHAR(1)))
    {
        int_proc(c,type);
        retval = LA_ERR;
    }
    return retval;
}

int float_dot(char** c, int* type)
{
    *type = FLOAT;
                           
    int i = strlen(*c)-1;
    int originalStrlen = i+1;
    int retval = ALL_PASS;
    
    do {
        getchar_check_mem(c, &i);
    } while(isdigit(CHAR(i)));

    if(i == originalStrlen)
    {

        retval = LA_ERR; 
    }

    if(CHAR(i) == 'E' || CHAR(i) == 'e')
    {
        if(retval == LA_ERR)
        {
            retval = float_e(c, type);
            if(retval != INTERNAL_ERR)
                retval = LA_ERR;
        }
        else
        {
            retval = float_e(c, type);
        }
    }
    return retval;
}

int float_e(char** c, int* type)
{
    *type = FLOAT;
    int i = strlen(*c)-1; 
    int retval = LA_ERR;
    
    getchar_check_mem(c, &i);

    if(CHAR(i) == '+')
    {
        CHAR(i) = getchar(); 
    } else if(CHAR(i) == '-') {
        getchar_check_mem(c, &i);
    }
    
    while(isdigit(CHAR(i)))
    {
        retval = ALL_PASS;
        getchar_check_mem(c, &i);
    }
    

    return retval;
}

int int_proc(char** c, int* type)
{
    *type = INT;
    int i = strlen(*c)-1;
    int retval = ALL_PASS;

    do {
        getchar_check_mem(c, &i);
    } while(isdigit(CHAR(i)));
    
    if(CHAR(i) == '.')
    {
        retval = float_dot(c, type);
    } else if (CHAR(i) == 'e' || CHAR(i) == 'E'){
        retval = float_e(c, type);
    }

    return retval;
}

int colon(char** c, int* type)
{
    *type = DECLARATION;
    int retval = ALL_PASS;
    CHAR(1) = getchar();
    if (CHAR(1) == '=')
    {
        CHAR(2) = getchar();
    } else {
        retval = LA_ERR;
    }
    return retval;
}

int semicolon(char** c, int* type)
{
    *type = SEMICOLON;
    CHAR(1) = getchar();
    return 0; 
}

int end_line(char** c, int* type)
{
    *type = END_LINE;
    
    CHAR(1)=getchar();
    return 0;
}

int less_than(char** c, int* type)     
{
    *type = LESS_THAN;
    CHAR(1) = getchar();
    if(CHAR(1) == '=')
    {
        *type = LESS_THAN_EQ;
        CHAR(2)=getchar();
    }
    return 0;
}

int equal(char** c, int* type)     
{
    *type = EQUAL;
    CHAR(1) = getchar();
    if(CHAR(1) == '='){
        *type = CMP_EQUAL;
        CHAR(2)=getchar();
    }
    return 0;
}

int more_than(char** c, int* type)
{
    *type = MORE_THAN;
    CHAR(1) = getchar();
    if(CHAR(1) == '=')
    {
        *type = MORE_THAN_EQ;
        CHAR(2)=getchar();
    }
    return 0;
}

int id_proc(char** c, int* type)
{
    *type = ID;
    int i = strlen(*c)-1;
    do {
        getchar_check_mem(c, &i);
    } while (isalpha(CHAR(i)) || isdigit(CHAR(i)) || (CHAR(i) == '_'));
    
    return 0;
}

int underscore(char** c, int* type)
{
    *type = UNDERSCORE;
    CHAR(1) = getchar();
    if (isdigit(CHAR(1)) || isalpha(CHAR(1)))
        id_proc(c, type);
    return 0;
}

int start_block(char** c, int* type)
{
    *type = START_BLOCK;
    CHAR(1) = getchar();
    return 0;
}

int end_block(char** c, int* type)
{
    *type = END_BLOCK;
    CHAR(1) = getchar();
    return 0;
}

void getchar_check_mem(char** c, int* i){
    check_mem(c,i);
    (*c)[(*i)] = getchar();
}

void check_mem(char**c,int*i)
{
    (*i)++;
    if ((*i % SIZE) == 0){    
        char* tmp = NULL;

        tmp = (char*)realloc(*c,sizeof(char)*((*i) + SIZE) + 1);
        
        if (tmp == NULL){
            free_and_exit(INTERNAL_ERR);
        } else {
            
            *c=tmp;
            tmp = NULL;
            for(int j = (*i); j<(*i)+SIZE+1; j++) {
                
                (*c)[j] = '\0';
            }
        }
        
    }
}
