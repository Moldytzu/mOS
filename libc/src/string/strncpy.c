#include <string.h>
#include <stdio.h>

char *strncpy(char *restrict s1,const char *restrict s2, size_t n)
{
    for(int i = 0; i < n; i++)
    {
        char c = *(s2++);

        if(!c)
            *(s1++) = 0;
        else
            *(s1++) = c;
    }
    return 0;
}