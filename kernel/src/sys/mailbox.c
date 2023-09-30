#include <sys/sys.h>

uint64_t mailbox(uint64_t call, uint64_t rdx, uint64_t r8, uint64_t r9, sched_task_t *task)
{
    switch (call)
    {
    case 0: // read last

        // read next mail
        mailbox_t *mail = mailReadNext(&task->mailbox);

        if (!mail)
            return 0;

        // allocate a virtual memory range
        void *virtualStart = vmaAllocatePage(task->virtualMemoryContext); // fixme: this isn't ok if contents >1 pages

        // map contents in virtual memory
        vmmMap(task->pageTable, virtualStart, mail->contents, VMM_ENTRY_RO | VMM_ENTRY_USER);

        // fixme: add returned pages to allocated buffer to prevent potential memory leaks in case the app doesn't free the mail
        // a new helper function would help

        return (uint64_t)virtualStart;
    default:
        return 0;
    }
}