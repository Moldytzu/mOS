#include <sys/sys.h>
#include <drv/input.h>

// input (rsi = device type)
uint64_t input(uint64_t deviceType, uint64_t rdx, uint64_t r8, uint64_t r9, sched_task_t *task)
{
    switch (deviceType)
    {
    case 0: // keyboard
        return kbGetLastKey();

    default:
        return 0;
    }
}
