#pragma once
#include <sys/sys.h>
#include <scheduler.h>

// write (rsi = buffer, rdx = count, r8 = fd)
void write(uint64_t syscallNumber, uint64_t buffer, uint64_t count, uint64_t returnAddress, uint64_t fd, uint64_t ignored, uint64_t r9, struct sched_task *task)
{
    if (fd == SYS_STDIN)
    {
        if (buffer < alignD(task->intrerruptStack.rsp, 4096) - 4096) // prevent a crash
            return;

        const char *charBuffer = (const char *)PHYSICAL(buffer); // get physical address of the buffer
        struct vt_terminal *t = vtGet(task->terminal);           // terminal of the task
        vtAppend(t, charBuffer, count);                          // append to the terminal
    }

    // todo: write to the file descriptor in vfs
}