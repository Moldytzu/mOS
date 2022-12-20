#pragma once
#include <misc/utils.h>
#include <fw/bootloader.h>

pstruct
{
    void *base;               // base address of the pool
    void *alloc;              // base address of the allocable memory
    uint64_t available;       // available bytes
    uint64_t used;            // used bytes
    uint64_t bitmapBytes;     // bytes used by the bitmap
    uint64_t lastBitmapIndex; // last bitmap index used in allocation
    uint64_t lastPageIndex;   // last page index used in allocation
    uint8_t lastMaskBit;      // last bit mask used in allocation
}
pmm_pool_t;

void *pmmPage();
void *pmmPages(uint64_t pages);

void pmmDeallocate(void *page);
void pmmDeallocatePages(void *page, uint64_t count);

pmm_pool_t pmmTotal();
pmm_pool_t *pmmPools();

void pmmInit();