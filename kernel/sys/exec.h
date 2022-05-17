#pragma once
#include <sys/sys.h>
#include <scheduler.h>
#include <elf.h>
#include <vt.h>

struct pack sys_exec_packet
{
    uint8_t shouldCreateNewTerminal;
    const char *enviroment;
    const char *pwd;
};

// exec (rsi = path, rdx = pid, r8 = packet)
void exec(uint64_t syscallNumber, uint64_t path, uint64_t pid, uint64_t returnAddress, uint64_t packet, uint64_t ignored, uint64_t r9, struct sched_task *task)
{
    if (path < TASK_BASE_ADDRESS || pid < alignD(task->intrerruptStack.rsp, 4096) || packet < alignD(task->intrerruptStack.rsp, 4096)) // prevent a crash
        return;

    struct sched_task *newTask = elfLoad(PHYSICAL(path));
    uint64_t *ret = PHYSICAL(pid);
    struct sys_exec_packet *input = PHYSICAL(packet);

    if (newTask == NULL) // failed to load the task
    {
        *ret = UINT64_MAX;
        return;
    }

    if (input->shouldCreateNewTerminal)
        newTask->terminal = vtCreate()->id; // create new tty and set the id to it's id
    else
        newTask->terminal = task->terminal; // set the parent's terminal id

    if (input->enviroment > (const char *)TASK_BASE_ADDRESS)
        memcpy(newTask->enviroment, PHYSICAL(input->enviroment), 4096); // copy the enviroment

    *ret = newTask->id; // set the pid
}