#include <sys/sys.h>
#include <mm/pmm.h>

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

        // we store the newly allocated pages' address in a buffer
        void *newPages[pages];
        for (int i = 0; i < pages; i++) // fixme: doing allocations between these iterations could lead to undefined behaviour.......
            newPages[i] = pmmPage();

        // map every page
        size_t virtualStart = 0; // virtual address of the starting byte of the newly allocated block
        for (size_t p = 0; p < pages; p++)
        {
            void *virtualPage = vmaAllocatePage(task->virtualMemoryContext); // newly allocated virtual page
            if (!virtualStart)
                virtualStart = (uint64_t)virtualPage;

            vmmMap(task->pageTable, virtualPage, newPages[p], VMM_ENTRY_RW | VMM_ENTRY_USER); // map the page in the virtual address space of the task
            pushUsedPage(task, newPages[p]);                                                  // push the page in the allocated pages array
            task->lastVirtualAddress += VMM_PAGE;                                             // point to next page
        }

        return virtualStart;
    }
    default:
        return SYSCALL_STATUS_UNKNOWN_OPERATION;
    }
}