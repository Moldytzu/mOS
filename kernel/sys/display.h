#pragma once
#include <sys/sys.h>
#include <drv/framebuffer.h>

// display (rsi = call, rdx = arg1, r8 = arg2)
void display(uint64_t call, uint64_t arg1, uint64_t arg2, uint64_t r9, struct sched_task *task)
{
    switch (call)
    {
    case 0: // display set mode
        if (arg1 == VT_DISPLAY_FB || arg1 == VT_DISPLAY_TTY0)
            vtSetMode(arg1); // set vt mode
        break;
    case 1: // display set resolution
        drv_type_framebuffer_s.requestedXres = arg1;
        drv_type_framebuffer_s.requestedYres = arg2;

        printks("set resolution to %d %d\n", drv_type_framebuffer_s.requestedXres, drv_type_framebuffer_s.requestedYres);
        break;
    default:
        break;
    }
}