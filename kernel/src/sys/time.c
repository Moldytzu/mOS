#include <sys/sys.h>
#include <sched/time.h>

uint64_t time(uint64_t call, uint64_t arg1, uint64_t arg2, uint64_t r9, sched_task_t *task)
{
    switch (call)
    {
    case 0: // get system uptime in nanoseconds
        if (!IS_MAPPED(arg1))
            return SYSCALL_STATUS_ERROR;

        uint64_t *arg1Ptr = PHYSICAL(arg1);
        *arg1Ptr = timeNanos();
        return SYSCALL_STATUS_OK;

    default:
        return SYSCALL_STATUS_UNKNOWN_OPERATION;
    }
}