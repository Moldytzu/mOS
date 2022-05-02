#pragma once
#include <sys/sys.h>
#include <scheduler.h>
#include <elf.h>
#include <vt.h>

// exec (rsi = path, rdx = newTerminal, r8 = pid address)
void exec(uint64_t syscallNumber, uint64_t path, uint64_t newTerminal, uint64_t returnAddress, uint64_t pid, uint64_t r9)
{
    struct sched_task *newTask = elfLoad(PHYSICAL(path));
    uint64_t *ret = PHYSICAL(pid);

    if (newTerminal)
        newTask->terminal = vtCreate()->id; // create new tty and set the id to it's id
    else
        newTask->terminal = schedulerGetCurrent()->terminal; // set the parent's terminal id

    *ret = newTask->id; // set the pid
}