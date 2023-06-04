#include <string.h>
#include <stdio.h>

char *strstr(const char *s1, const char *s2)
{
    size_t targetLen = strlen(s2);
    size_t len = strlen(s1);

    if (targetLen > len)
        return NULL;

    if (!targetLen)
        return (char *)s1;

    do
    {
        len = strlen(s1);
        if(memcmp(s1,s2, targetLen) == 0)
            return (char *)s1;

        s1++;
    } while (len > targetLen);
    
    return NULL;
}