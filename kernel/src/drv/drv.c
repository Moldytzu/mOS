#include <drv/drv.h>
#include <cpu/idt.h>
#include <mm/blk.h>
#include <sched/scheduler.h>
#include <misc/logger.h>

drv_context_input_t *inputCtx;
uint16_t inputIdx = 0;

drv_context_fb_t *fbCtx;
drv_context_fb_t fbRef;
uint16_t fbIdx = 0;

void drvInit()
{
    // allocate the contexts
    fbCtx = blkBlock(sizeof(drv_context_fb_t) * DRV_MAX_CONTEXTS);
    inputCtx = blkBlock(sizeof(drv_context_input_t) * DRV_MAX_CONTEXTS);
}

void drvExit(uint32_t drv)
{
    // clear contexts used by the drv
    for (int i = 0; i < DRV_MAX_CONTEXTS; i++)
    {
        if (fbCtx[i].pid == drv)
            zero(&fbCtx[i], sizeof(drv_context_fb_t));

        if (inputCtx[i].pid == drv)
            zero(&inputCtx[i], sizeof(drv_context_input_t));
    }

    idtClearRedirect(drv);
    logInfo("drv: %s exits", schedGet(drv)->name);
}

void drvUpdateReference(uint32_t type, void *context)
{
    switch (type)
    {
    case DRV_TYPE_FB:
        memcpy(&fbRef, context, sizeof(drv_context_fb_t));
        for (int i = 0; i < fbIdx; i++)
        {
            fbCtx[i].requestedXres = fbRef.requestedXres;
            fbCtx[i].requestedYres = fbRef.requestedYres;
        }

        break;

    default:
        break;
    }
}

void *drvRegister(uint32_t drv, uint32_t type)
{
    // return a new context
    logInfo("drv: %s registred as type %d driver", schedGet(drv)->name, type);

    switch (type)
    {

    case DRV_TYPE_FB:
    {
        if (fbIdx == DRV_MAX_CONTEXTS) // hell nah man (todo: reallocate every time)
            return NULL;

        // generate the context from the active one (if there is)
        fbCtx[fbIdx].pid = drv;
        if (drvQueryActive(DRV_TYPE_FB))
        {
            drv_context_fb_t *active = drvQueryActive(DRV_TYPE_FB);
            fbCtx[fbIdx].requestedXres = active->requestedXres;
            fbCtx[fbIdx].requestedYres = active->requestedYres;
        }

        return &fbCtx[fbIdx++];
        break;
    }

    case DRV_TYPE_INPUT:
    {
        if (inputIdx == DRV_MAX_CONTEXTS) // hell nah man (todo: reallocate every time)
            return NULL;

        inputCtx[inputIdx].pid = drv;
        return &inputCtx[inputIdx++];
        break;
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
        // search the one that's valid (has a pid set) and that which has the up to date resolution set
        for (int i = 0; i < fbIdx; i++)
        {
            if (fbCtx[i].pid && fbCtx[i].currentXres == fbRef.requestedXres && fbCtx[i].currentYres == fbRef.requestedYres)
                return &fbCtx[i];
        }

        // return first valid if we didn't find one
        for (int i = 0; i < fbIdx; i++)
        {
            if (fbCtx[i].pid)
                return &fbCtx[i];
        }

        return NULL; // we don't have any active context

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

void drvQueryContexts(uint32_t type, void **contexts, uint32_t *size)
{
    switch (type)
    {

    case DRV_TYPE_FB:
    {
        *contexts = fbCtx;
        *size = fbIdx;
        break;
    }

    case DRV_TYPE_INPUT:
    {
        *contexts = inputCtx;
        *size = inputIdx;
        break;
    }

    default:
        break;
    }
}