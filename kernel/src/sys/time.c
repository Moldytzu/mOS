#include <sys/sys.h>
#include <sched/time.h>

uint64_t time(uint64_t call, uint64_t arg1, uint64_t arg2, uint64_t r9, uint64_t r10, sched_task_t *task)
{
    switch (call)
    {
    case 0: // get system uptime in nanoseconds
        return timeNanos();

    default:
        return SYSCALL_STATUS_UNKNOWN_OPERATION;
    }
}