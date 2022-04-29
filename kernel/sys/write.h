#pragma once
#include <sys/sys.h>
#include <scheduler.h>

// write (rsi = buffer, rdx = count, r8 = fd)
void write(uint64_t syscallNumber, uint64_t buffer, uint64_t count, uint64_t returnAddress, uint64_t fd, uint64_t r9)
{
    // todo: check the fd and then write, if it's 1 then write to the terminal's buffer
    const char *charBuffer = (const char *)vmmGetPhys(schedulerGetCurrent()->pageTable, (void *)buffer); // get physical address of the buffer
    for (size_t i = 0; i < count; i++)
        printk("%c", charBuffer[i]);
}