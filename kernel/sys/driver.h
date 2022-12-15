#pragma once
#include <sys/sys.h>
#include <mm/pmm.h>
#include <mm/heap.h>

static bool firstTime = false;

// driver (rsi = call, rdx = arg1, r8 = arg2)
void driver(uint64_t call, uint64_t arg1, uint64_t arg2, uint64_t r9, struct sched_task *task)
{
    if (!task->isDriver && task->id != 1) // unprivileged
        return;

    if(!firstTime)
    {
        // clear the context structures
        zero(&drv_type_input_s, sizeof(drv_type_input_s));

        firstTime = !firstTime;
    }

    switch (call)
    {
    case 0: // driver start
        if (!INBOUNDARIES(arg1) && !INBOUNDARIES(arg2))
            return;

        char *path = expandPath(PHYSICAL(arg1), task);
        uint64_t *pid = PHYSICAL(arg2);

        if (!path)
            return;

        elfLoad(path, 0, 0, true); // start the driver
        break;

    case 1: // driver announce
        if(!INBOUNDARIES(arg2))
            return;

        uint64_t *retStruct = PHYSICAL(arg2);

        switch (arg1)
        {
        case 1: // input
            *retStruct = (uint64_t)&drv_type_input_s;
            break;
        
        default:
            *retStruct = 0;
            break;
        }

        break;

    case 2: // flush struct updates
        switch (arg1) // type
        {
        case 1: // input
            inputFlush();
        default:
            break;
        }

        break;

    case 3: // redirect idt gate
        if (!INBOUNDARIES(arg1))
            return;

        idtRedirect((void *)arg1, arg2, task->id);

        break;
    case 4: // reset idt gate

        // todo: disable the idt redirection
        break;

    default:
        break;
    }
}