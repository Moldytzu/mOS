#include <sys/sys.h>
#include <misc/logger.h>

// pid (rsi = pid, rdx = info, r8 = retVal/inputVal)
uint64_t pid(uint64_t pid, uint64_t info, uint64_t retVal, uint64_t r9, sched_task_t *task)
{
    uint64_t *retAddr = PHYSICAL(retVal);
    sched_task_t *t = schedGet(pid);
    if (!t) // check if the task exists
    {
        *retAddr = UINT64_MAX;
        return SYSCALL_STATUS_ERROR;
    }

    switch (info)
    {
    case 0:                  // get pid state
        *retAddr = t->state; // give the state in which that pid is in
        return SYSCALL_STATUS_OK;

    case 1:                     // get pid enviroment
        if (!IS_MAPPED(retVal)) // available only in the allocated memory
            return SYSCALL_STATUS_ERROR;
        memcpy(PHYSICAL(retVal), t->enviroment, 4096); // copy the enviroment
        return SYSCALL_STATUS_OK;

    case 2:                     // set pid enviroment
        if (!IS_MAPPED(retVal)) // available only in the allocated memory
            return SYSCALL_STATUS_ERROR;
        memcpy(t->enviroment, PHYSICAL(retVal), 4096); // copy the enviroment
        return SYSCALL_STATUS_OK;

    case 3:                     // get current pid
        if (!IS_MAPPED(retVal)) // prevent crashing
            return SYSCALL_STATUS_ERROR;
        *retAddr = task->id; // the id
        return SYSCALL_STATUS_OK;

    case 4:                     // get current working directory
        if (!IS_MAPPED(retVal)) // available only in the allocated memory
            return SYSCALL_STATUS_ERROR;

        task->cwd[0] = '/'; // make sure the cwd starts with /

        memcpy(PHYSICAL(retVal), task->cwd, min(strlen(task->cwd), 512)); // copy the buffer
        return SYSCALL_STATUS_OK;

    case 5:                     // set current working directory
        if (!IS_MAPPED(retVal)) // available only in the allocated memory
            return SYSCALL_STATUS_ERROR;

        zero(task->cwd, 512);
        memcpy(task->cwd, PHYSICAL(retVal), min(strlen(PHYSICAL(retVal)), 512)); // copy the buffer
        return SYSCALL_STATUS_OK;

    default:
        return SYSCALL_STATUS_OK;
    }
}