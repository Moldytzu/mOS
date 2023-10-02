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
    default:
        return SYSCALL_STATUS_UNKNOWN_OPERATION;
    }
}