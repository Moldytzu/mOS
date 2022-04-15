#pragma once
#include <utils.h>
#include <bootloader.h>

struct mm_pool
{
    uint64_t total;         // total memory
    uint64_t available;     // available memory
    uint64_t used;          // used memory
    bool full;              // if there isn't any available memory available
    void *base;             // base address of the physical memory in the pool
    void *allocableBase;    // base address of the allocable memory in the pool
    size_t pageIndex;       // page index
    size_t bitmapReserved;  // bytes reserved for the bitmap
    size_t bitmapByteIndex; // byte in the bitmap
    uint8_t bitmapBitIndex; // bit in the byte in the bitmap
    uint8_t *bitmapBase;    // base pointer of the bitmap
};

bool mmIsFreePage(struct mm_pool *pool, size_t page);
void mmDeallocatePage(void *address);
void mmDeallocatePagePool(struct mm_pool *pool, void *address);
void *mmAllocatePage();
void *mmAllocatePages(size_t pages);
void *mmAllocatePagePool(struct mm_pool *pool);
void *mmAllocatePagePoolIndex(struct mm_pool *pool, size_t page);
void *mmAllocatePagesPool(struct mm_pool *pool, size_t pages);
void pmmInit();
struct mm_pool *mmGetPools();
struct mm_pool mmGetTotal();