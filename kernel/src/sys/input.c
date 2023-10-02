#include <sys/sys.h>
#include <drv/input.h>

// input (rsi = device type)
uint64_t input(uint64_t deviceType, uint64_t rdx, uint64_t r8, uint64_t r9, uint64_t r10, sched_task_t *task)
{
    switch (deviceType)
    {
    case 0: // keyboard
        return kbGetLastKey();

    case 1: // mouse
        if (!IS_MAPPED(rdx) || !IS_MAPPED(r8))
            return SYSCALL_STATUS_ERROR;

        mouseGet((uint16_t *)PHYSICAL(rdx), (uint16_t *)PHYSICAL(r8));

        return SYSCALL_STATUS_OK;
    default:
        return 0;
    }
}
