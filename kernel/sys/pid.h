#pragma once
#include <sys/sys.h>
#include <scheduler.h>

// pid (rsi = pid, rdx = info, r8 = retVal)
void pid(uint64_t syscallNumber, uint64_t pid, uint64_t info, uint64_t returnAddress, uint64_t retVal, uint64_t r9)
{
    uint64_t *retAddr = PHYSICAL(retVal);
    switch (info)
    {
    case 0: // get pid state
        struct sched_task *task = schedulerGet(pid);
        if(!task) // couldn't find the task
            *retAddr = 0xFF; 
        *retAddr = task->state; // give the state in which that pid is in
        break;
    default:
        break;
    }
}