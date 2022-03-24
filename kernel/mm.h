#pragma once
#include <utils.h>
#include <bootloader.h>

struct pack mm_pool
{
    uint64_t total;         // total memory
    uint64_t available;     // available memory
    uint64_t used;          // used memory
    bool full;              // if there isn't any available memory available
    void *base;             // base address of the physical memory in the pool
    void *allocableBase;    // base address of the allocable memory in the pool
    size_t bitmapReserved;  // bytes reserved for the bitmap
};

bool mmIsFreePage(struct mm_pool *pool, size_t pageIndex);
void mmDeallocatePage(void *address);
void mmDeallocatePagePool(struct mm_pool *pool, void *address);
void *mmAllocatePage();
void *mmAllocatePagePool(struct mm_pool *pool);
void mmInit();
struct mm_pool *mmGetPools();
struct mm_pool mmGetTotal();