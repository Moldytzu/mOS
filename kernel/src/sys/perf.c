#include <sys/sys.h>

// perf (rsi = call, rdx = arg1, r8 = arg2)
uint64_t perf(uint64_t call, uint64_t arg1, uint64_t arg2, uint64_t r9, uint64_t r10, sched_task_t *task)
{
    switch (call)
    {
    case 0: // get memory info
        if (!IS_MAPPED(arg1) || !IS_MAPPED(arg2))
            return SYSCALL_STATUS_ERROR;

        pmm_pool_t total = pmmTotal();
        *(uint64_t *)PHYSICAL(arg1) = total.used;
        *(uint64_t *)PHYSICAL(arg2) = total.available;

        return SYSCALL_STATUS_OK;
    case 1: // get cpu info
        if (!IS_MAPPED(arg1))
            return SYSCALL_STATUS_ERROR;

        *(uint64_t *)PHYSICAL(arg1) = smpCores();
        return SYSCALL_STATUS_OK;
    default:
        return SYSCALL_STATUS_UNKNOWN_OPERATION;
    }
}
