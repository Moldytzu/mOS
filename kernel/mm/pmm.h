#pragma once
#include <misc/utils.h>
#include <fw/bootloader.h>

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

void mmDeallocatePages(void *address, size_t pages);
void mmDeallocatePage(void *address);
void *mmAllocatePage();
void *mmAllocatePages(size_t pages);
void pmmInit();
struct mm_pool *mmGetPools();
struct mm_pool mmGetTotal();