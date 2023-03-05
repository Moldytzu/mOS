#pragma once
#include <sys/sys.h>

// pid (rsi = pid, rdx = info, r8 = retVal/inputVal)
void pid(uint64_t pid, uint64_t info, uint64_t retVal, uint64_t r9, struct sched_task *task)
{
    uint64_t *retAddr = PHYSICAL(retVal);
    struct sched_task *t = schedulerGet(pid);
    if (!t) // check if the task exists
    {
        *retAddr = UINT64_MAX;
        return;
    }

    switch (info)
    {
    case 0:                  // get pid state
        *retAddr = t->state; // give the state in which that pid is in
        break;
    case 1:                        // get pid enviroment
        if (!INBOUNDARIES(retVal)) // available only in the allocated memory
            break;
        memcpy(PHYSICAL(retVal), t->enviroment, 4096); // copy the enviroment
        break;
    case 2:                        // set pid enviroment
        if (!INBOUNDARIES(retVal)) // available only in the allocated memory
            break;
        memcpy(t->enviroment, PHYSICAL(retVal), 4096); // copy the enviroment
        break;
    case 3:                        // get current pid
        if (!INBOUNDARIES(retVal)) // prevent crashing
            return;
        *retAddr = task->id; // the id
        break;
    case 4:                        // get current working directory
        if (!INBOUNDARIES(retVal)) // available only in the allocated memory
            break;
        memcpy(PHYSICAL(retVal), task->cwd, 512); // copy the buffer
        break;
    case 5:                        // set current working directory
        if (!INBOUNDARIES(retVal)) // available only in the allocated memory
            break;
        memcpy(task->cwd, PHYSICAL(retVal), 512); // copy the buffer
        break;
    default:
        break;
    }
}