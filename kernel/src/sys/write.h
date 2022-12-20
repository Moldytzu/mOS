#pragma once
#include <sys/sys.h>
#include <fs/vfs.h>

// write (rsi = buffer, rdx = count, r8 = fd)
void write(uint64_t buffer, uint64_t count, uint64_t fd, uint64_t r9, struct sched_task *task)
{
    if (!INBOUNDARIES(buffer) && count > 1) // prevent a crash
        return;

    const char *charBuffer = (const char *)PHYSICAL(buffer); // get physical address of the buffer

    if (task->isDriver)
    {
        // todo: use the kernel logger that doesn't exist
        printks("%s", charBuffer);
    }

    if (fd == SYS_STDIN)
    {
        struct vt_terminal *t = vtGet(task->terminal); // terminal of the task
        vtAppend(t, charBuffer, count);                // append to the terminal

        return;
    }

    vfsWrite(fd, (void *)charBuffer, count, 0); // write to the descriptor

    // todo: also support writing to another terminal
}