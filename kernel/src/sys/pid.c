#include <sys/sys.h>
#include <misc/logger.h>

// pid (rsi = pid, rdx = call, r8 = arg1, r9 = arg2)
uint64_t pid(uint64_t pid, uint64_t call, uint64_t arg1, uint64_t arg2, uint64_t r10, sched_task_t *task)
{
    sched_task_t *t = schedGet(pid);

    switch (call)
    {
    case 0: // get pid state
        if (!t)
            return UINT64_MAX;
        else
            return t->state;

        // TODO: do boundary checks

    case 1: // get pid enviroment
        if (!IS_MAPPED(arg1) || !t)
            return SYSCALL_STATUS_ERROR;

        memcpy(PHYSICAL(arg1), t->enviroment, 4096); // copy the enviroment
        return SYSCALL_STATUS_OK;

    case 2: // set pid enviroment
        if (!IS_MAPPED(arg1) || !t)
            return SYSCALL_STATUS_ERROR;

        memcpy(t->enviroment, PHYSICAL(arg1), 4096); // copy the enviroment
        return SYSCALL_STATUS_OK;

    case 3: // get current pid
        return task->id;

    case 4: // get current working directory
        if (!IS_MAPPED(arg1))
            return SYSCALL_STATUS_ERROR;

        task->cwd[0] = '/'; // make sure the cwd starts with /

        memcpy(PHYSICAL(arg1), task->cwd, min(strlen(task->cwd), 512)); // copy the buffer
        return SYSCALL_STATUS_OK;

    case 5: // set current working directory
        if (!IS_MAPPED(arg1))
            return SYSCALL_STATUS_ERROR;

        zero(task->cwd, 512);
        memcpy(task->cwd, PHYSICAL(arg1), min(strlen(PHYSICAL(arg1)), 512)); // copy the buffer
        return SYSCALL_STATUS_OK;

    default:
        return SYSCALL_STATUS_UNKNOWN_OPERATION;
    }
}