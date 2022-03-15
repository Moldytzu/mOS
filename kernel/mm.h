#pragma once
#include <utils.h>
#include <bootloader.h>

struct mm_info
{
    uint64_t total;     // total memory
    uint64_t available; // available memory
    uint64_t used;      // used memory
    void *base;         // base address of the physical memory
};

void mmInit();
struct mm_info mmGetInfo();