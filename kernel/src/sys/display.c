#include <sys/sys.h>
#include <drv/framebuffer.h>
#include <drv/drv.h>
#include <subsys/vt.h>

// display (rsi = call, rdx = arg1, r8 = arg2)
uint64_t display(uint64_t call, uint64_t arg1, uint64_t arg2, uint64_t r9, sched_task_t *task)
{
    switch (call)
    {
    case 0: // display set mode
        if (arg1 == VT_DISPLAY_FB || arg1 == VT_DISPLAY_TTY0)
        {
            vtSetMode(arg1); // set vt mode
            return SYSCALL_STATUS_OK;
        }
        else
            return SYSCALL_STATUS_UNKNOWN_OPERATION;

    case 1: // display set resolution
        drv_context_fb_t newCtx;
        newCtx.requestedXres = arg1;
        newCtx.requestedYres = arg2;
        drvUpdateReference(DRV_TYPE_FB, &newCtx);
        return SYSCALL_STATUS_OK; // todo: make this return error if no fb contexts exist

    case 2: // display get resolution
        if (!IS_MAPPED(arg1) || !IS_MAPPED(arg2))
            return SYSCALL_STATUS_ERROR;

        // pass information from the global framebuffer
        *(uint64_t *)PHYSICAL(arg1) = framebufferGet().width;
        *(uint64_t *)PHYSICAL(arg2) = framebufferGet().height;

        return SYSCALL_STATUS_OK;

    default:
        return SYSCALL_STATUS_UNKNOWN_OPERATION;
    }
}