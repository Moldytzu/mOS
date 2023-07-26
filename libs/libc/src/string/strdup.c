#include <string.h>
#include <stdio.h>
#include <stdlib.h>

char *strdup( const char *src )
{
    void *buffer = malloc(strlen(src) + 1);
    memset(buffer, 0, strlen(src) + 1);
    memcpy(buffer, src, strlen(src));
    return buffer;
}