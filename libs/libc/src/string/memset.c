#include <string.h>
#include <stdint.h>

void *memset(void *str, int c, size_t n)
{
    for (; n; n--, str++)
        *(uint8_t *)str = c;
    return str;
}
