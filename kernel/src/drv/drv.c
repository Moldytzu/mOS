#include <drv/drv.h>
#include <cpu/idt.h>
#include <mm/blk.h>
#include <sched/scheduler.h>

drv_context_input_t *inputCtx;
uint16_t inputIdx;

drv_context_fb_t *fbCtx;
drv_context_fb_t fbRef;
uint16_t fbIdx;

void drvInit()
{
    // allocate the contexts
    fbCtx = blkBlock(sizeof(drv_context_fb_t) * DRV_MAX_CONTEXTS);
    inputCtx = blkBlock(sizeof(drv_context_input_t) * DRV_MAX_CONTEXTS);
    zero(fbCtx, sizeof(drv_context_fb_t) * DRV_MAX_CONTEXTS);
    zero(inputCtx, sizeof(drv_context_input_t) * DRV_MAX_CONTEXTS);

    fbIdx = 0;
    inputIdx = 0;
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
    printks("drv: %s registred as type %d driver\n", schedulerGet(drv)->name, type);

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
        for (int i = 0; i < fbIdx; i++)
        {
            if (fbCtx[fbIdx].currentXres == fbRef.requestedXres && fbCtx[fbIdx].currentYres == fbRef.requestedYres)
                return &fbCtx[fbIdx];
        }

        if (fbIdx) // return first as a fallback
            return &fbCtx[0];

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