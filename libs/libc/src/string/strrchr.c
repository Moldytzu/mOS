#include <string.h>
#include <stdio.h>

char *strrchr(const char *s, int c)
{
    while(*s++)
        if(*s == c)
            return (char *)s;
    
    return NULL;
}