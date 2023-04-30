#include <sys/sys.h>
#include <elf/elf.h>
#include <subsys/vt.h>
#include <mm/pmm.h>

pstruct
{
    uint8_t shouldCreateNewTerminal;
    const char *enviroment;
    const char *cwd;
    int argc;
    char **argv;
}
sys_exec_packet_t;

// exec (rsi = path, rdx = pid, r8 = packet)
void exec(uint64_t path, uint64_t pid, uint64_t packet, uint64_t r9, sched_task_t *task)
{
    if (!INBOUNDARIES(path) || !INBOUNDARIES(pid) || !INBOUNDARIES(packet)) // prevent a crash
        return;

    uint64_t *ret = PHYSICAL(pid);
    sys_exec_packet_t *input = PHYSICAL(packet);
    char *execPath = expandPath(PHYSICAL(path), task); // expand the path

    if (execPath == NULL || !strlen(execPath)) // the path doesn't exist
    {
        *ret = 0;
        return;
    }

    // convert virtual addresses to physical addresses
    if (input->argc && INBOUNDARIES(input->argv))
    {
        input->argv = PHYSICAL(input->argv);

        for (int i = 0; i < input->argc; i++)
            if (INBOUNDARIES(input->argv[i]))
                input->argv[i] = PHYSICAL(input->argv[i]);
    }

    sched_task_t *newTask = elfLoad(execPath, input->argc, input->argv, false); // do the loading
    *ret = newTask->id;                                                         // set the pid

    if (input->shouldCreateNewTerminal)
        newTask->terminal = vtCreate()->id; // create new tty and set the id to it's id
    else
        newTask->terminal = task->terminal; // set the parent's terminal id

    if (input->enviroment)
        memcpy(newTask->enviroment, PHYSICAL(input->enviroment), strlen(PHYSICAL(input->enviroment)) + 1); // copy the enviroment

    if (input->cwd)
        memcpy(newTask->cwd, PHYSICAL(input->cwd), strlen(PHYSICAL(input->cwd)) + 1); // copy the initial working directory
}