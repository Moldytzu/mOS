#include <lai/core.h>
#include <lai/host.h>
#include <mm/blk.h>
#include <main/panic.h>
#include <misc/logger.h>

// helper functions for lai
void laihost_log(int level, const char *msg)
{
    logInfo("lai log %d: %s", level, msg);
}

__attribute__((noreturn)) void laihost_panic(const char *msg)
{
    panick(msg);
    while (1)
        ;
}

void *laihost_malloc(size_t size)
{
    return blkBlock(size);
}

void *laihost_realloc(void *ptr, size_t newsize, size_t oldsize)
{
    return blkReallocate(ptr, newsize);
}

void laihost_free(void *ptr, size_t size)
{
    blkDeallocate(ptr);
}