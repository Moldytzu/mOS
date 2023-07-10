#include <sys/sys.h>
#include <drv/input.h>

// input (rsi = device type, rdx = return pointer)
void input(uint64_t deviceType, uint64_t returnPtr, uint64_t r8, uint64_t r9, sched_task_t *task)
{
    if (!IS_MAPPED(returnPtr)) // prevent crashing
        return;

    char *ret = (char *)PHYSICAL(returnPtr); // return pointer

    switch (deviceType)
    {
    case 0: // keyboard
        *ret = kbGetLastKey();
        break;

    default:
        *ret = '\0'; // set to 0 meaning error
        break;
    }
}
