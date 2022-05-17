#pragma once
#include <sys/sys.h>
#include <scheduler.h>
#include <vfs.h>

// read (rsi = buffer, rdx = count, r8 = fd)
void read(uint64_t syscallNumber, uint64_t buffer, uint64_t count, uint64_t returnAddress, uint64_t fd, uint64_t ignored, uint64_t r9, struct sched_task *task)
{
    if (!INBOUNDARIES(buffer)) // prevent a crash
        return;

    void *charBuffer = PHYSICAL(buffer); // get physical address of the buffer

    vfsRead(fd, charBuffer, count, 0); // read from the vfs

    // todo: also support reading from another terminal
}