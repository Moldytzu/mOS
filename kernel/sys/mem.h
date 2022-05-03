#pragma once
#include <sys/sys.h>
#include <scheduler.h>
#include <pmm.h>

// mem (rsi = call, rdx = arg1, r8 = arg2)
void mem(uint64_t syscallNumber, uint64_t call, uint64_t arg1, uint64_t returnAddress, uint64_t arg2, uint64_t r9, struct sched_task *task)
{
    switch (call)
    {
    case 0: // mem allocate
        if (arg1 == 0)
            return;
        void *pageAddress = mmAllocatePage();                                       // allocate a page
        uint32_t index = task->allocatedIndex++;                                    // get the index
        task->allocated[index] = pageAddress;                                       // keep evidence of it
        vmmMap(task->pageTable, task->lastVirtualAddress, pageAddress, true, true); // map the address
        *(uint64_t *)PHYSICAL(arg1) = (uint64_t)task->lastVirtualAddress;           // give the application the virtual address
        task->lastVirtualAddress += 4096;                                           // increment the virtual address
        break;
    default:
        break;
    }
}