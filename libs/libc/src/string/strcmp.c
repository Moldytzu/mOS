#include <string.h>
#include <stdint.h>

int strcmp(const char *str1, const char *str2)
{
    while (*str1)
    {
        if (*str2 != *str1)
            break;

        str1++;
        str2++;
    }

    return *(const unsigned char *)str1 - *(const unsigned char *)str2;
}