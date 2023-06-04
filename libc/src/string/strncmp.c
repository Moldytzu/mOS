#include <string.h>
#include <stdint.h>
#include <stdio.h>

int strncmp(const char *s1, const char *s2, size_t n)
{
    while(*s1 != 0 && *s2 != 0)
    {
        if(*s1 != *s2 || !(n--))
            break;
    }
    return *s1 - *s2;
}