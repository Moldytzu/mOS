#pragma once
#include <sys/sys.h>
#include <scheduler.h>
#include <elf.h>
#include <vt.h>

// exec (rsi = path, rdx = newTerminal, r8 = pid, r9 = enviroment)
void exec(uint64_t syscallNumber, uint64_t path, uint64_t newTerminal, uint64_t returnAddress, uint64_t pid, uint64_t ignored, uint64_t enviroment, struct sched_task *task)
{
    struct sched_task *newTask = elfLoad(PHYSICAL(path));
    uint64_t *ret = PHYSICAL(pid);

    if (newTask == NULL) // failed to load the task
    {
        *ret = UINT64_MAX;
        return;
    }

    if (newTerminal)
        newTask->terminal = vtCreate()->id; // create new tty and set the id to it's id
    else
        newTask->terminal = task->terminal; // set the parent's terminal id

    if(enviroment)
         memcpy(newTask->enviroment, PHYSICAL(enviroment), 4096); // copy the enviroment

    *ret = newTask->id; // set the pid
}