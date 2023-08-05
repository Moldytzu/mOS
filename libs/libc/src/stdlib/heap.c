#include <stdlib.h>
#include <mos/sys.h>
#include <stdio.h>
#include <assert.h>

// todo: rewrite this

void *malloc(size_t s)
{
    return (void *)sys_mem(SYS_MEM_ALLOCATE, s / 4096 + 1, 0); // allocate
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