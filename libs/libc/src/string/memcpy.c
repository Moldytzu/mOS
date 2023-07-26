#include <string.h>
#include <stdint.h>

void *memcpy(void *restrict s1,
             const void *restrict s2, size_t n)
{
    for (size_t i = 0; i < n; i++)
        ((uint8_t *)s1)[i] = ((uint8_t *)s2)[i];
    return s1;
}
