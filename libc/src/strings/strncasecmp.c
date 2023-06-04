#include <strings.h>
#include <stdio.h>
#include <ctype.h>

int strncasecmp(const char *s1, const char *s2, size_t n)
{
    while(*s1 != 0 && *s2 != 0)
    {
        if(tolower(*s1) != tolower(*s2) || !(n--))
            break;
    }
    return *s1 - *s2;
}