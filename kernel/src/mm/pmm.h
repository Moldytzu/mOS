#pragma once
#include <misc/utils.h>
#include <fw/bootloader.h>

#define PMM_PAGE 4096

typedef struct
{
    void *base;                  // base address of the pool
    void *alloc;                 // base address of the allocable memory
    uint64_t size;               // maximum allocable bytes
    uint64_t available;          // available bytes
    uint64_t used;               // used bytes
    uint64_t bitmapBytes;        // bytes used by the bitmap
    uint64_t lastAllocatedIndex; // index of the last allocated page

    spinlock_t lock; // spinlock
} pmm_pool_t;

void pmmBenchmark();
void pmmDbgDump();
void *pmmPage();
void *pmmPages(uint64_t pages);
void *pmmReallocate(void *ptr, uint64_t oldSize, uint64_t newSize);

void pmmDeallocate(void *page);
void pmmDeallocatePages(void *page, uint64_t count);

pmm_pool_t pmmTotal();
pmm_pool_t *pmmPools();

void pmmInit();