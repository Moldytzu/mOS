#pragma once
#include <sys/sys.h>
#include <fs/vfs.h>

// (rsi = fd)
void close(uint64_t fd, uint64_t rbx, uint64_t r8, uint64_t r9, sched_task_t *task)
{
    if (fd == 0)
        return;

    vfsClose(fd); // close the node
}