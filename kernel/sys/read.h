#pragma once
#include <sys/sys.h>
#include <scheduler.h>

// read (rsi = buffer, rdx = count, r8 = fd)
void read(uint64_t syscallNumber, uint64_t buffer, uint64_t count, uint64_t returnAddress, uint64_t fd, uint64_t r9)
{
    // stub
}