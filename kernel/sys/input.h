#pragma once
#include <sys/sys.h>
#include <subsys/input.h>

// input (rsi = device type, rdx = return pointer)
void input(uint64_t deviceType, uint64_t returnPtr, uint64_t r8, uint64_t r9, struct sched_task *task)
{
    if (!INBOUNDARIES(returnPtr)) // prevent crashing
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
