#include <drv/drv.h>
#include <drv/input.h>
#include <subsys/vt.h>
#include <sched/scheduler.h>
#include <cpu/smp.h>

struct vt_terminal *startTerminal, *currentTerminal;

// todo: implement mouse support

// pop last key
char kbGetLastKey()
{
    return vtkbGet(vtGet(schedGetCurrent(smpID())->terminal)); // get last key of the terminal
}

// get the buffer
char *kbGetBuffer()
{
    return (char *)vtGet(schedGetCurrent(smpID())->terminal)->buffer; // get the buffer
}

// initialize the subsystem
void inputInit()
{
    startTerminal = vtGet(0);
}

// flush struct changes
void inputFlush()
{
    // append the scan codes from all input devices to the terminal buffers
    drv_context_input_t *inputs;
    uint32_t idx;
    drvQueryContexts(DRV_TYPE_INPUT, (void *)&inputs, &idx);

    for (size_t i = 0; i < idx; i++)
    {
        for (int j = 0; j < min(strlen(inputs[i].keys), 16); j++)
        {
            char c = inputs[i].keys[j];

            // append the key to every terminal in a loop
            currentTerminal = startTerminal;
            while (currentTerminal)
            {
                vtkbAppend(currentTerminal, c);
                currentTerminal = currentTerminal->next;
            }
        }

        zero(inputs[i].keys, 16);
    }
}