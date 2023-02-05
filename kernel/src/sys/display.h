#pragma once
#include <sys/sys.h>
#include <drv/framebuffer.h>
#include <drv/drv.h>

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
        drv_context_fb_t newCtx = {.requestedXres = arg1, .requestedYres = arg2};
        drvUpdateReference(DRV_TYPE_FB, &newCtx);
        break;
    case 2: // display get resolution
        if (!INBOUNDARIES(arg1) || !INBOUNDARIES(arg2) || !drvQueryActive(DRV_TYPE_FB))
            return;

        *(uint64_t *)PHYSICAL(arg1) = ((drv_context_fb_t *)drvQueryActive(DRV_TYPE_FB))->currentXres;
        *(uint64_t *)PHYSICAL(arg2) = ((drv_context_fb_t *)drvQueryActive(DRV_TYPE_FB))->currentYres;
    default:
        break;
    }
}