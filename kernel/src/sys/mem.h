#pragma once
#include <sys/sys.h>
#include <mm/pmm.h>

// mem (rsi = call, rdx = arg1, r8 = arg2)
void mem(uint64_t call, uint64_t arg1, uint64_t arg2, uint64_t r9, struct sched_task *task)
{
    switch (call)
    {
    case 0:                      // mem allocate
        if (!INBOUNDARIES(arg1)) // prevent crashing
            return;

        // todo: make it so the application can allocate smaller chunks of memory (maybe implement an allocator for each task based on the one present in mm/blk.c?)

        void *page = pmmPage();                                                            // allocate a page
        zero(page, VMM_PAGE);                                                              // clear it
        vmmMap(task->pageTable, task->lastVirtualAddress, page, true, true, false, false); // map it

        *(uint64_t *)PHYSICAL(arg1) = (uint64_t)task->lastVirtualAddress; // give the application the virtual address
        task->lastVirtualAddress += 4096;                                 // increment the last virtual address

        if (task->allocatedIndex == ((task->allocatedBufferPages * VMM_PAGE) / 8) - 1) // reallocation needed when we overflow
        {
            task->allocated = pmmReallocate(task->allocated, task->allocatedBufferPages, task->allocatedBufferPages + 1);
            task->allocatedBufferPages++;
        }

        task->allocated[task->allocatedIndex++] = page; // keep evidence of the page
        break;
    case 1: // mem info
        if (!INBOUNDARIES(arg1) || !INBOUNDARIES(arg2))
            return;

        pmm_pool_t total = pmmTotal();
        *(uint64_t *)PHYSICAL(arg1) = total.used;
        *(uint64_t *)PHYSICAL(arg2) = total.available;
    default:
        break;
    }
}