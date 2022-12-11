#pragma once
#include <sys/sys.h>
#include <mm/pmm.h>
#include <mm/heap.h>

// driver (rsi = call, rdx = arg1, r8 = arg2)
void driver(uint64_t call, uint64_t arg1, uint64_t arg2, uint64_t r9, struct sched_task *task)
{
    if (!task->isDriver && task->id != 1) // unprivileged
        return;

    switch (call)
    {
    case 0: // driver start
        if (!INBOUNDARIES(arg1) && !INBOUNDARIES(arg2))
            return;

        char *path = expandPath(PHYSICAL(arg1), task);
        uint64_t *pid = PHYSICAL(arg2);

        if (!path)
            return;

        elfLoad(path, 0, 0, true); // start the driver

        break;
    default:
        break;
    }
}