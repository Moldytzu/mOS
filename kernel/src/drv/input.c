#include <misc/logger.h>
#include <drv/drv.h>
#include <drv/input.h>
#include <fw/bootloader.h>
#include <subsys/vt.h>
#include <sched/scheduler.h>
#include <cpu/smp.h>

struct vt_terminal *startTerminal, *currentTerminal;
uint16_t mouseX, mouseY;

extern struct limine_framebuffer framebuffer; // structure implemented in drv/framebuffer.c, holds global information about the GPU framebuffer

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
        for (uint32_t j = 0; j < min(strlen(inputs[i].keys), 16); j++)
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

        zero(inputs[i].keys, 16);

        // add the offsets
        mouseX += inputs[i].mouseX;
        mouseY += inputs[i].mouseY;

        if (mouseX < 0)
            mouseX = 0;

        if (mouseY < 0)
            mouseY = 0;

        // clamp the values
        if (mouseX >= framebuffer.width - 1)
            mouseX = framebuffer.width - 1;

        if (mouseY >= framebuffer.height - 1)
            mouseY = framebuffer.height - 1;

        // reset the context
        inputs[i].mouseX = inputs[i].mouseY = 0;
    }
}