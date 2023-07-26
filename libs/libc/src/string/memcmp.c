#include <string.h>
#include <stdint.h>

int memcmp(const void *str1, const void *str2, size_t n)
{
    for (size_t i = 0; i < n; i++, str1++, str2++)
    {
        if (*(uint8_t *)str1 != *(uint8_t *)str2)
            return *(uint8_t *)str1 - *(uint8_t *)str2;
    }
    return 0;
}