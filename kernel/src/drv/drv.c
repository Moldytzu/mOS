#include <drv/drv.h>
#include <cpu/idt.h>
#include <mm/blk.h>
#include <sched/scheduler.h>

drv_context_fb_t *fb;
drv_context_fb_t fbRef;
uint16_t fbIdx;

void drvInit()
{
    // allocate the contexts
    fb = blkBlock(sizeof(drv_context_fb_t) * DRV_MAX_CONTEXTS);
    zero(fb, sizeof(drv_context_fb_t) * DRV_MAX_CONTEXTS);

    fbIdx = 0;
}

void drvExit(uint32_t drv)
{
    // clear contexts used by the drv
    for (int i = 0; i < DRV_MAX_CONTEXTS; i++)
    {
        if (fb[i].pid == drv)
            zero(&fb[i], sizeof(drv_context_fb_t));
    }

    idtClearRedirect(drv);
    printks("drv: %s exits\n", schedulerGet(drv)->name);
}

void drvUpdateReference(uint32_t type, void *context)
{
    switch (type)
    {
    case DRV_TYPE_FB:
        memcpy(&fbRef, context, sizeof(drv_context_fb_t));
        for (int i = 0; i < fbIdx; i++)
        {
            fb[i].requestedXres = fbRef.requestedXres;
            fb[i].requestedYres = fbRef.requestedYres;
        }

        break;

    default:
        break;
    }
}

void *drvRegister(uint32_t drv, uint32_t type)
{
    // return a new context
    printks("drv: %s registred as type %d driver\n", schedulerGet(drv)->name, type);

    switch (type)
    {

    case DRV_TYPE_FB:
    {
        if (fbIdx == DRV_MAX_CONTEXTS) // hell nah man (todo: reallocate every time)
            return NULL;

        // generate the context from the active one (if there is)
        fb[fbIdx].pid = drv;
        if (drvQueryActive(DRV_TYPE_FB))
        {
            drv_context_fb_t *active = drvQueryActive(DRV_TYPE_FB);
            fb[fbIdx].requestedXres = active->requestedXres;
            fb[fbIdx].requestedYres = active->requestedYres;
        }

        fbIdx++;

        return &fb[fbIdx - 1];
    }

    default:
        break;
    }

    return NULL;
}

void *drvQueryActive(uint32_t type)
{
    // return the most up-to-date context
    switch (type)
    {
    case DRV_TYPE_FB:
        for (int i = 0; i < fbIdx; i++)
        {
            if (fb[fbIdx].currentXres == fbRef.requestedXres && fb[fbIdx].currentYres == fbRef.requestedYres)
                return &fb[fbIdx];
        }

        if (fbIdx) // return first as a fallback
            return &fb[0];

        break;

    default:
        break;
    }

    return NULL;
}

void drvSocketCreate(uint32_t drv, uint32_t sockID)
{
}

void drvQuerySockets(uint32_t drv, uint32_t *sockets)
{
}

void drvQueryContexts(uint32_t type, void **contexts)
{
}