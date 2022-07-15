#include <string.h>
#include <stdint.h>

size_t strlen(const char *str)
{
    size_t i = 0;
    for (; str[i]; i++)
        ;
    return i;
}

int memcmp(const void *str1, const void *str2, size_t n)
{
    for (size_t i = 0; i < n; i++, str1++, str2++)
    {
        if (*(uint8_t *)str1 != *(uint8_t *)str2)
            return *(uint8_t *)str1 - *(uint8_t *)str2;
    }
    return 0;
}

void *memcpy(void *dest, const char *src, size_t n)
{
    for (size_t i = 0; i < n; i++)
        ((uint8_t *)dest)[i] = ((uint8_t *)src)[i];
    return dest;
}

void *memset(void *str, int c, size_t n)
{
    for (; n; n--, str++)
        *(uint8_t *)str = c;
    return str;
}

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