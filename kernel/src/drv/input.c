#include <misc/logger.h>
#include <drv/drv.h>
#include <drv/input.h>
#include <drv/framebuffer.h>
#include <subsys/vt.h>
#include <sched/scheduler.h>
#include <cpu/smp.h>

struct vt_terminal *startTerminal, *currentTerminal;
uint16_t mouseX, mouseY;

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
    mouseX = mouseY = 0;
    startTerminal = vtGet(0);
}

// flush struct changes
void inputFlush()
{
    // append the scan codes from all input devices to the terminal buffers
    drv_context_input_t *inputs;
    uint32_t idx;
    drvQueryContexts(DRV_TYPE_INPUT, (void *)&inputs, &idx);

    for (uint32_t i = 0; i < idx; i++)
    {
        for (uint32_t j = 0; j < min(strlen(inputs[i].keys), 64); j++)
        {
            char c = inputs[i].keys[j];

            // append the key to every terminal in a loop
            currentTerminal = startTerminal;

            while (currentTerminal)
            {
                vtkbAppend(currentTerminal, c);

                if (!currentTerminal->next) // for some reason this fixes a crash
                    break;

                currentTerminal = currentTerminal->next;
            }
        }

        zero(inputs[i].keys, 64);

        // generate preliminary values
        int tempX = mouseX + inputs[i].mouseX;
        int tempY = mouseY + inputs[i].mouseY;

        // process them to become proper screen coordinates
        if (tempX < 0)
            tempX = 0;

        if (tempY < 0)
            tempY = 0;

        if (tempX >= framebufferGet().width - 1)
            tempX = framebufferGet().width - 1;

        if (tempY >= framebufferGet().height - 1)
            tempY = framebufferGet().height - 1;

        // save the processed values
        mouseX = tempX;
        mouseY = tempY;

        // reset the context
        inputs[i].mouseX = inputs[i].mouseY = 0;
    }
}

// get mouse coordinates
void mouseGet(uint16_t *x, uint16_t *y)
{
    *x = mouseX;
    *y = mouseY;
}