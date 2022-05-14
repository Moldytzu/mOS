#pragma once
#include <sys/sys.h>
#include <scheduler.h>

// pid (rsi = pid, rdx = info, r8 = retVal/inputVal)
void pid(uint64_t syscallNumber, uint64_t pid, uint64_t info, uint64_t returnAddress, uint64_t retVal, uint64_t ignored, uint64_t r9, struct sched_task *task)
{
    if (retVal < alignD(task->intrerruptStack.rsp, 4096)) // prevent crashing
        return;

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
    case 1:                                            // get pid enviroment
        memcpy(PHYSICAL(retVal), t->enviroment, 4096); // copy the enviroment
        break;
    case 2:                                            // set pid enviroment
        memcpy(t->enviroment, PHYSICAL(retVal), 4096); // copy the enviroment
        break;
    case 3:                  // get current pid
        *retAddr = task->id; // the id
        break;
    default:
        break;
    }
}