#pragma once
#include <sys/sys.h>
#include <scheduler.h>

// write (rsi = buffer, rdx = count, r8 = fd)
void write(uint64_t syscallNumber, uint64_t buffer, uint64_t count, uint64_t returnAddress, uint64_t fd, uint64_t r9)
{
    if (fd == SYS_STDIN)
    {
        const char *charBuffer = (const char *)vmmGetPhys(schedulerGetCurrent()->pageTable, (void *)buffer); // get physical address of the buffer
        struct vt_terminal *t = vtGet(schedulerGetCurrent()->terminal);                                      // terminal of the task
        vtAppend(t, charBuffer, count);                                                                      // append to the terminal
    }

    // todo: write to the file descriptor in vfs
}