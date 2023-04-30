#include <stdlib.h>
#include <mos/sys.h>
#include <stdio.h>
#include <assert.h>

// todo: rewrite this

void *malloc(size_t s)
{
    uint64_t address;
    sys_mem(SYS_MEM_ALLOCATE, (uint64_t)&address, 0); // allocate
    return (void *)address;
}

void free(void *ptr)
{
    return;
}