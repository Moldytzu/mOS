#pragma once
#include <sys/sys.h>
#include <elf/elf.h>
#include <subsys/vt.h>
#include <mm/pmm.h>

struct pack sys_exec_packet
{
    uint8_t shouldCreateNewTerminal;
    const char *enviroment;
    const char *cwd;
    int argc;
    char **argv;
};

// exec (rsi = path, rdx = pid, r8 = packet)
void exec(uint64_t path, uint64_t pid, uint64_t packet, uint64_t r9, struct sched_task *task)
{
    if (!INBOUNDARIES(path) || !INBOUNDARIES(pid) || !INBOUNDARIES(packet)) // prevent a crash
        return;

    int argc = 0;
    char **argv = NULL;

    uint64_t *ret = PHYSICAL(pid);
    struct sys_exec_packet *input = PHYSICAL(packet);

    if (input->argc)
    {
        if (!INBOUNDARIES(input->argv))
            return;

        input->argv = PHYSICAL(input->argv); // get phyisical address of the argv

        argc = input->argc;
        argv = input->argv;

        // convert addresses to physical
        for (int i = 0; i < argc; i++)
        {
            if (!INBOUNDARIES(argv[i])) // check
                return;

            argv[i] = PHYSICAL(argv[i]);
        }
    }

    char *inputPath = expandPath(PHYSICAL(path), task); // expand the path

    if (inputPath == NULL) // if the path is null then it doesn't exist
    {
        *ret = 0; // fail
        mmDeallocatePage(inputPath);
        return;
    }

    struct sched_task *newTask = elfLoad(inputPath, argc, argv); // do the loading

    if (newTask == NULL) // failed to load the task
    {
        *ret = UINT64_MAX;
        return;
    }

    if (input->shouldCreateNewTerminal)
        newTask->terminal = vtCreate()->id; // create new tty and set the id to it's id
    else
        newTask->terminal = task->terminal; // set the parent's terminal id

    if (input->enviroment > (char *)alignD(task->intrerruptStack.rsp, 4096) - 4096)
        memcpy(newTask->enviroment, PHYSICAL(input->enviroment), strlen(PHYSICAL(input->enviroment)) + 1); // copy the enviroment

    if (input->cwd > (char *)alignD(task->intrerruptStack.rsp, 4096) - 4096)
        memcpy(newTask->cwd, PHYSICAL(input->cwd), strlen(PHYSICAL(input->cwd)) + 1); // copy the initial working directory

    *ret = newTask->id; // set the pid
    mmDeallocatePage(inputPath);
}