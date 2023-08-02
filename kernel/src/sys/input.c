#include <sys/sys.h>
#include <drv/input.h>

// input (rsi = device type, rdx = return pointer)
uint64_t input(uint64_t deviceType, uint64_t returnPtr, uint64_t r8, uint64_t r9, sched_task_t *task)
{
    if (!IS_MAPPED(returnPtr)) // prevent crashing
        return SYSCALL_STATUS_ERROR;

    char *ret = (char *)PHYSICAL(returnPtr); // return pointer

    switch (deviceType)
    {
    case 0: // keyboard
        *ret = kbGetLastKey();
        return *ret;

    default:
        *ret = '\0'; // set to 0 meaning error
        return *ret;
    }
}
