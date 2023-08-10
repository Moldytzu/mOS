#include <sys/sys.h>
#include <mm/pmm.h>

#define ADDRESSES_IN_PAGES(x) (x * VMM_PAGE / sizeof(uint64_t))
#define PAGES_IN_ADDRESSES(x) (x * sizeof(uint64_t) / VMM_PAGE)
#define RESERVED_PAGES 8

// mem (rsi = call, rdx = arg1, r8 = arg2)
uint64_t mem(uint64_t call, uint64_t arg1, uint64_t arg2, uint64_t r9, sched_task_t *task)
{
    switch (call)
    {
    case 0: // mem allocate
    {
        size_t pages = arg1; // required pages to allocate
        if (!pages)          // make sure we don't allocate null
            pages = 1;

        if (pmmTotal().available < (pages + RESERVED_PAGES) * 4096) // make sure we don't run in an out of memory panic
            return 0;

        size_t virtualStart = task->lastVirtualAddress; // virtual address of the starting byte of the newly allocated block

        // we store the newly allocated pages' address in a buffer
        void *newPages[pages];
        for (int i = 0; i < pages; i++)
            newPages[i] = pmmPage();

        // map every page
        for (size_t p = 0; p < pages; p++)
        {
            vmmMap(task->pageTable, (void *)task->lastVirtualAddress, newPages[p], VMM_ENTRY_RW | VMM_ENTRY_USER); // map the page in the virtual address space of the task

            if (task->allocatedIndex + 1 >= ADDRESSES_IN_PAGES(task->allocatedBufferPages))                                 // check if we can not store the newly allocated page's address
                task->allocated = pmmReallocate(task->allocated, task->allocatedBufferPages, ++task->allocatedBufferPages); // do the reallocation

            task->allocated[task->allocatedIndex++] = newPages[p]; // store the address to free up later
            task->lastVirtualAddress += VMM_PAGE;                  // point to next page
        }

        return virtualStart;
    }
    default:
        return SYSCALL_STATUS_UNKNOWN_OPERATION;
    }
}