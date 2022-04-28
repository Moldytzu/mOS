#include <stdlib.h>

size_t strlen(const char *str)
{
    size_t i = 0;
    for(;str[i];i++);
    return i;
}