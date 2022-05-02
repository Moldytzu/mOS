#pragma once
#include <sys/sys.h>
#include <scheduler.h>

// exit (rsi = exit status)
void exit(uint64_t syscallNumber, uint64_t status, uint64_t rdx, uint64_t returnAddress, uint64_t r8, uint64_t r9)
{
#ifdef K_SYSCALL_DEBUG
    printks("syscall: exit status %d\n\r", status);
#endif
    schedulerKill(schedulerGetCurrent()->id); // kill the task
}
