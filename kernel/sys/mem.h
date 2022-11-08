#pragma once
#include <sys/sys.h>
#include <mm/pmm.h>
#include <mm/heap.h>

// mem (rsi = call, rdx = arg1, r8 = arg2)
void mem(uint64_t call, uint64_t arg1, uint64_t arg2, uint64_t r9, struct sched_task *task)
{
    switch (call)
    {
    case 0:                      // mem allocate
        if (!INBOUNDARIES(arg1)) // prevent crashing
            return;

        void *page = pmmPage();                                              // allocate a page
        memset64(page, 0, VMM_PAGE / sizeof(uint64_t));                      // clear it
        vmmMap(task->pageTable, task->lastVirtualAddress, page, true, true); // map it

        *(uint64_t *)PHYSICAL(arg1) = (uint64_t)task->lastVirtualAddress; // give the application the virtual address
        task->lastVirtualAddress += 4096;                                 // increment the last virtual address

        task->allocated[task->allocatedIndex++] = page;                                                     // keep evidence of the page
        task->allocated = (void **)realloc(task->allocated, (task->allocatedIndex + 1) * sizeof(uint64_t)); // make the allocated array bigger
        break;
    default:
        break;
    }
}