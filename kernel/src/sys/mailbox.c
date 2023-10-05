#include <sys/sys.h>

uint64_t mailbox(uint64_t call, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, sched_task_t *task)
{
    switch (call)
    {
    case 0: // read last
        // read next mail
        mailbox_t *mail = mailReadNext(&task->mailbox);

        if (!mail)
            return 0;

        // fixme: this isn't ok if contents >1 pages

        // allocate a virtual memory range
        void *virtualStart = vmaAllocatePage(task->virtualMemoryContext);

        // map contents in virtual memory
        vmmMap(task->pageTable, virtualStart, mail->contents, VMM_ENTRY_RO | VMM_ENTRY_USER);

        sysPushUsedPage(task, PHYSICAL(virtualStart)); // push physical page to clean up
        mailFreeBox(mail);                             // free metadata of mail

        return (uint64_t)virtualStart;

    case 1: // compose
        sched_task_t *destination = schedGet(arg1);
        if (!destination)
            return SYSCALL_STATUS_ACCESS_DENIED;

        if (!IS_MAPPED(arg2))
            return SYSCALL_STATUS_ACCESS_DENIED;

        mailCompose(&destination->mailbox, task->id, arg4, PHYSICAL(arg2), arg3); // fixme: we shouldn't trust arg3 here

        return SYSCALL_STATUS_OK;

    case 2: // free

        // fixme: this isn't ok if contents >1 pages
        // fixme: this doesn't check if page is mail or other ordinary page

        uint64_t physical = (uint64_t)PHYSICAL(arg1);

        // check if it is actually allocated
        uint64_t *allocated = (uint64_t *)task->allocated;
        bool used = false;
        for (int i = 0; i < task->allocatedIndex; i++)
        {
            if (allocated[i] == physical)
            {
                allocated[i] = 0;
                used = true;
            }
        }

        if (!used)
            return SYSCALL_STATUS_ACCESS_DENIED;

        // unmap page
        vmmUnmap(task->pageTable, (void *)arg1);

        // deallocate page
        pmmDeallocate((void *)physical);

        return SYSCALL_STATUS_OK;

    default:
        return SYSCALL_STATUS_UNKNOWN_OPERATION;
    }
}