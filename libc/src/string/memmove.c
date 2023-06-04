#include <string.h>
#include <stdio.h>
#include <stdio.h>

void *memmove(void *s1, const void *s2, size_t n)
{
    uint8_t *from = (uint8_t *)s2;
    uint8_t *to = (uint8_t *)s1;

    if (from == to || n == 0)
        return s1;

    if (to > from && to - from < n)
    {
        for (size_t i = n - 1; i >= 0; i--)
            to[i] = from[i];
        return s1;
    }
    if (from > to && from - to < n)
    {
        for (size_t i = 0; i < n; i++)
            to[i] = from[i];
        return s1;
    }
    memcpy(s1, s2, n);
    return s1;
}