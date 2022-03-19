#pragma once
#include <utils.h>
#include <bootloader.h>

struct mm_info
{
    uint64_t total;      // total memory
    uint64_t available;  // available memory
    uint64_t used;       // used memory
    void *base;          // base address of the physical memory
    void *allocableBase; // base address of the allocable memory
};

struct mm_pool
{
    uint64_t total;      // total memory
    uint64_t available;  // available memory
    uint64_t used;       // used memory
    void *base;          // base address of the physical memory in the pool
    void *allocableBase; // base address of the allocable memory in the pool
};

void mmDeallocatePage(void *address);
void *mmAllocatePage();
void mmInit();
struct mm_info mmGetInfo();
struct mm_pool *mmGetPools();