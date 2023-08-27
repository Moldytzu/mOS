#include <sys/sys.h>

uint64_t mailbox(uint64_t rsi, uint64_t rdx, uint64_t r8, uint64_t r9, sched_task_t *task)
{
    // todo: check for rsi to be valid. if it is then do the deallocation of a mail. check before if it is mapped and if it is in the mailbox

    // returns last mail
    mailbox_t *mail = mailReadNext(&task->mailbox);

    if (!mail)
        return 0;

    for (int i = 0; i < mailPages(mail); i++)
        vmmMap(task->pageTable, (void *)((uint64_t)mail + i * 4096), (void *)((uint64_t)mail + i * 4096), VMM_ENTRY_USER);

    // fixme: add returned pages to allocated buffer to prevent potential memory leaks in case the app doesn't free the mail
    // a new helper function would help

    return (uint64_t)mail;
}