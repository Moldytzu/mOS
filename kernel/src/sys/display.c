#include <sys/sys.h>
#include <drv/framebuffer.h>
#include <drv/drv.h>
#include <vt/vt.h>

// display (rsi = call, rdx = arg1, r8 = arg2)
uint64_t display(uint64_t call, uint64_t arg1, uint64_t arg2, uint64_t r9, sched_task_t *task)
{
    switch (call)
    {
    case 0: // display set mode
        if (arg1 == VT_DISPLAY_FB || arg1 == VT_DISPLAY_TTY0 || arg1 == VT_DISPLAY_FB_DOUBLE_BUFFERED)
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

    case 3: // display map framebuffer
        if (!IS_MAPPED(arg1))
            return 0;

        // fixme: we don't remap when the framebuffer changes size
        // fixme: wouldn't it be a better idea to have a buffer just for userspace?

        uint64_t *pitch = PHYSICAL(arg1);

        struct limine_framebuffer fb = framebufferGetBack();

        void *start = fb.address;
        for (size_t p = 0; p < fb.pitch * fb.height; p += VMM_PAGE)
            vmmMap(task->pageTable, (void *)TASK_BASE_FRAMEBUFFER + p, start + p, VMM_ENTRY_RW | VMM_ENTRY_USER | VMM_ENTRY_WRITE_THROUGH);

        *pitch = fb.pitch;

        return TASK_BASE_FRAMEBUFFER;

    case 4: // update userspace framebuffer
        if (vtGetMode() == VT_DISPLAY_FB_DOUBLE_BUFFERED)
            framebufferUpdate();
        return 0;
    default:
        return SYSCALL_STATUS_UNKNOWN_OPERATION;
    }
}