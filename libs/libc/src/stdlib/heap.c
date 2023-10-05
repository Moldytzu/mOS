#include <stdlib.h>
#include <mos/sys.h>
#include <stdio.h>
#include <assert.h>

// todo: rewrite this

void *malloc(size_t s)
{
    return sys_mem_allocate(s / 4096 + 1);
}

void free(void *ptr)
{
    return;
}

void *calloc(size_t nmemb, size_t size)
{
    return malloc(size * nmemb);
}

void *realloc(void *ptr, size_t size)
{
    return malloc(size);
}