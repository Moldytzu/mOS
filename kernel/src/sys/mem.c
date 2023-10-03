#include <sys/sys.h>
#include <mm/pmm.h>

// mem (rsi = call, rdx = arg1, r8 = arg2)
uint64_t mem(uint64_t call, uint64_t arg1, uint64_t arg2, uint64_t r9, uint64_t r10, sched_task_t *task)
{
    switch (call)
    {
    case 0: // mem allocate
    {
        size_t pages = arg1; // required pages to allocate
        if (!pages)          // make sure we don't allocate null
            pages = 1;

        if (pmmTotal().available <= (pages + RESERVED_PAGES) * 4096) // make sure we don't run in an out of memory panic
        {
            logWarn("sys: %s tried to use more RAM (%d kB) than available (%d kB)", task->name, pages * 4096 / 1024, pmmTotal().available / 1024);
            exit(UINT64_MAX, 0, 0, 0, 0, task); // kill task
        }

        // map every page
        size_t virtualStart = 0; // virtual address of the starting byte of the newly allocated block
        for (size_t p = 0; p < pages; p++)
        {
            volatile void *virtualPage = vmaAllocatePage(task->virtualMemoryContext); // newly allocated virtual page
            volatile void *phyiscalPage = pmmPage();
            if (!virtualStart)
                virtualStart = (uint64_t)virtualPage;

            vmmMap(task->pageTable, (void *)virtualPage, (void *)phyiscalPage, VMM_ENTRY_RW | VMM_ENTRY_USER); // map the page in the virtual address space of the task
            sysPushUsedPage(task, (void *)phyiscalPage);                                                       // push the page in the allocated pages array
        }

        return virtualStart;
    }
    default:
        return SYSCALL_STATUS_UNKNOWN_OPERATION;
    }
}