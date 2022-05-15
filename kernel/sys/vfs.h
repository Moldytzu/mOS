#pragma once
#include <sys/sys.h>
#include <scheduler.h>
#include <vfs.h>

// vfs (rsi = call, rdx = arg1, r8 = retVal)
void vfs(uint64_t syscallNumber, uint64_t call, uint64_t arg1, uint64_t returnAddress, uint64_t retVal, uint64_t ignored, uint64_t r9, struct sched_task *task)
{
    if (retVal < alignD(task->intrerruptStack.rsp, 4096)) // prevent crashing
        return;

    uint64_t *retAddr = PHYSICAL(retVal);

    switch (call)
    {
    case 0: // file path exists
        if (arg1 < alignD(task->intrerruptStack.rsp, 4096))
            *retAddr = false;
        uint64_t fd = vfsOpen(PHYSICAL(arg1)); // open
        *retAddr = fd > 0;                     // if the fd is valid then the file exists
        vfsClose(fd);                          // close
    default:
        break;
    }
}