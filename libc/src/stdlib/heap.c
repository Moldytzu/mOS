#include <stdlib.h>
#include <mos/sys.h>
#include <stdio.h>
#include <assert.h>

// todo: rewrite this

void *malloc(size_t s)
{
    uint64_t address;
    sys_mem(SYS_MEM_ALLOCATE, (uint64_t)&address, s / 4096 + 1); // allocate
    return (void *)address;
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