#pragma once
#include <utils.h>

struct pack mm_pagetable
{
    unsigned present : 1; // exists
    unsigned writable : 1; // writable
    unsigned user : 1; // accessible in userspace
    unsigned writethrough : 1; // write directly to RAM
    unsigned accessed : 1; // set by the CPU when reading or writing
    unsigned dirty : 1; // set by the CPU when writing
    unsigned hugepage : 1; // huge pages
    unsigned reserved : 4;
    uint64_t physical : 40; // physical memory
    unsigned reserved2 : 11;
    unsigned noexecute : 1; // forbid executing code
};

void vmmInit();