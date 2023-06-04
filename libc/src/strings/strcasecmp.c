#include <strings.h>
#include <stdio.h>
#include <ctype.h>

int strcasecmp(const char *s1, const char *s2)
{
    while(*s1 != 0 && *s2 != 0)
    {
        if(tolower(*s1) != tolower(*s2))
            break;
    }
    return *s1 - *s2;
}