#pragma once
#include <sys/sys.h>
#include <scheduler.h>
#include <input.h>

// input (rsi = device type, rdx = return pointer)
void input(uint64_t syscallNumber, uint64_t deviceType, uint64_t returnPtr, uint64_t returnAddress, uint64_t r8, uint64_t r9)
{
    char *ret = (char *)vmmGetPhys(schedulerGetCurrent()->pageTable,(void *)returnPtr); // return pointer

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
