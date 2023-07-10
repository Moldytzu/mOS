#include <sys/sys.h>
#include <mm/pmm.h>

#define ADDRESSES_IN_PAGES(x) (x * VMM_PAGE / sizeof(uint64_t))
#define PAGES_IN_ADDRESSES(x) (x * sizeof(uint64_t) / VMM_PAGE)
#define RESERVED_PAGES 8

// mem (rsi = call, rdx = arg1, r8 = arg2)
void mem(uint64_t call, uint64_t arg1, uint64_t arg2, uint64_t r9, sched_task_t *task)
{
    switch (call)
    {
    case 0:                      // mem allocate
        if (!IS_MAPPED(arg1)) // prevent crashing
            return;

        size_t pages = arg2; // pages to allocate
        if (!pages)
            pages = 1;

        if(pmmTotal().available < (pages + RESERVED_PAGES) * 4096) // let some memory free so we don't crash
            return;

        void *page = pmmPages(pages); // allocate the pages

        // map pages
        for (int i = 0; i < pages; i++)
            vmmMap((void *)task->pageTable, (void *)(task->lastVirtualAddress + i * 4096), (void *)((uint64_t)page + i * 4096), VMM_ENTRY_RW | VMM_ENTRY_USER);

        *(uint64_t *)PHYSICAL(arg1) = task->lastVirtualAddress; // give the application the virtual address
        task->lastVirtualAddress += 4096 * pages;               // increment the last virtual address

        size_t addresses = ADDRESSES_IN_PAGES(task->allocatedBufferPages); // addresses we can store

        // check for possible overflow
        if (task->allocatedIndex + pages >= addresses)
        {
            size_t newPages = PAGES_IN_ADDRESSES(pages) + 1;
            task->allocated = pmmReallocate(task->allocated, task->allocatedBufferPages, task->allocatedBufferPages + newPages);
            task->allocatedBufferPages += newPages;
        }

        // store addresses so we can clean up later
        for (int i = 0; i < pages; i++)
            task->allocated[task->allocatedIndex++] = (void *)((uint64_t)page + i * 4096); 
        break;
    default:
        break;
    }
}