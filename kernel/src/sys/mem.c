#include <sys/sys.h>
#include <mm/pmm.h>

// mem (rsi = call, rdx = arg1, r8 = arg2)
void mem(uint64_t call, uint64_t arg1, uint64_t arg2, uint64_t r9, sched_task_t *task)
{
    switch (call)
    {
    case 0:                      // mem allocate
        if (!INBOUNDARIES(arg1)) // prevent crashing
            return;

        size_t pages = arg2; // pages to allocate
        if (!pages)
            pages = 1;

        void *page = pmmPages(pages); // allocate the pages

        // map pages
        for (int i = 0; i < pages; i++)
            vmmMap((void *)task->pageTable, (void *)(task->lastVirtualAddress + i * 4096), (void *)((uint64_t)page + i * 4096), VMM_ENTRY_RW | VMM_ENTRY_USER);

        *(uint64_t *)PHYSICAL(arg1) = task->lastVirtualAddress; // give the application the virtual address
        task->lastVirtualAddress += 4096 * pages;               // increment the last virtual address

        if (task->allocatedIndex == ((task->allocatedBufferPages * VMM_PAGE) / 8) - pages) // reallocation needed when we overflow (todo: here we might overflow)
        {
            task->allocated = pmmReallocate(task->allocated, task->allocatedBufferPages, task->allocatedBufferPages + 1);
            task->allocatedBufferPages++;
        }

        for (int i = 0; i < pages; i++)
            task->allocated[task->allocatedIndex++] = (void *)((uint64_t)page + i * 4096); // keep evidence of the page
        break;
    default:
        break;
    }
}