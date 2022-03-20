#pragma once
#include <utils.h>
#include <bootloader.h>

struct mm_pool
{
    uint64_t total;      // total memory
    uint64_t available;  // available memory
    uint64_t used;       // used memory
    void *base;          // base address of the physical memory in the pool
    void *allocableBase; // base address of the allocable memory in the pool
    uint8_t *bitmapByte; // the byte in the bitmap
    size_t bitmapIndex;  // the index in the bitmap
    bool full;           // if there isn't any available memory available
};

void mmDeallocatePage(void *address);
void mmDeallocatePagePool(struct mm_pool *pool, void *address);
void *mmAllocatePage();
void *mmAllocatePagePool(struct mm_pool *pool);
void mmInit();
struct mm_pool *mmGetPools();