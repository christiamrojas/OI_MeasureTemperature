#ifndef STR_H
#define STR_H

#include "Arduino.h"
#include "IPAddress.h"

bool str_searchchar(char *p, char search, int *n);
bool str_getstring(char *pOut,char *pIn,int *n,char c);
bool str_getip(char *pin, uint8_t *octet);

#endif