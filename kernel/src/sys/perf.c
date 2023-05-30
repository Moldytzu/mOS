#include <sys/sys.h>

// perf (rsi = call, rdx = arg1, r8 = arg2)
void perf(uint64_t call, uint64_t arg1, uint64_t arg2, uint64_t r9, sched_task_t *task)
{
    switch (call)
    {
    case 0: // get memory info
        if (!INBOUNDARIES(arg1) || !INBOUNDARIES(arg2))
            return;

        pmm_pool_t total = pmmTotal();
        *(uint64_t *)PHYSICAL(arg1) = total.used;
        *(uint64_t *)PHYSICAL(arg2) = total.available;
        break;
    case 1: // get cpu info
        if (!INBOUNDARIES(arg1))
            return;

        *(uint64_t *)PHYSICAL(arg1) = smpCores();
        break;
    default:
        break;
    }
}
