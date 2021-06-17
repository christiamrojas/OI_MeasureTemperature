#include "Str.h"

bool str_searchchar(char *p, char search, int *n)
{
    *n = 0;

    do{
        if( (*p) == search)     return true;
        if( (*p++) == 0)        return false;
        (*n)++;
    }while(true);

    return false;
}

bool str_getstring(char *pOut,char *pIn,int *n,char c)
{        
    if(str_searchchar(pIn,c,n)==false)  return false;
    strncpy(pOut,pIn,*n);
    pOut[*n] = 0;    
    (*n)++;
    return true;
}

bool str_getip(char *pin, uint8_t *octet)
{
    uint16_t num = 0;
    uint8_t dots = 0;

    while(*pin)
    {
        char c = *pin++;
        if( (c>='0') && (c<='9'))
        {
            num = num*10 + c - '0';
            if(num>255) return false;
        }
        else if (c=='.')
        {
            if(dots==3) return false;
            octet[dots++] = num;
            num = 0;        
        }
        else 
            return false; 
    }

    if(dots!=3) return false; 
    octet[3] = num; 
    return true; 
}