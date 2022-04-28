#pragma once
#include <sys/sys.h>
#include <scheduler.h>

// write (rsi = buffer, rdx = count, r8 = fd
void write(uint64_t syscallNumber, uint64_t rsi, uint64_t rdx, uint64_t returnAddress, uint64_t r8, uint64_t r9)
{
    // todo: check the fd and then print, if it's 1 then write to the terminal's buffer
    const char *buffer = vmmGetPhys(schedulerGetCurrent()->pageTable, (void *)rsi); // get physical address of the buffer
    for (size_t i = 0; i < rdx; i++)
        printk("%c", buffer[i]);
}