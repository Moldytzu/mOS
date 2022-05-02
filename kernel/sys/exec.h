#pragma once
#include <sys/sys.h>
#include <scheduler.h>
#include <elf.h>

// exec (rsi = path, rdx = newTerminal)
void exec(uint64_t syscallNumber, uint64_t path, uint64_t newTerminal, uint64_t returnAddress, uint64_t r8, uint64_t r9)
{
    elfLoad(vmmGetPhys(schedulerGetCurrent()->pageTable,(void *)path));
}