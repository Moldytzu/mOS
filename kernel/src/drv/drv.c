#include <drv/drv.h>
#include <cpu/idt.h>
#include <mm/blk.h>
#include <mm/pmm.h>
#include <sched/scheduler.h>
#include <misc/logger.h>

#define PAGES_PER_CONTEXT_ARRAY (1)
#define MAX_CONTEXTS_PER_ARRAY ((PAGES_PER_CONTEXT_ARRAY * PMM_PAGE) / sizeof(uint64_t))

// fixme: try to phase out this way of handling devices
// todo: implement a better way of abstracting devices (maybe use the unix way but with a quirk????????????)

drv_context_input_t **inputContexts;
uint16_t inputIndex = 0;

drv_context_fb_t **fbContexts;
drv_context_fb_t fbReference;
uint16_t fbIndex = 0;

void drvInit()
{
    // allocate the contexts
    inputContexts = pmmPages(PAGES_PER_CONTEXT_ARRAY);
    fbContexts = pmmPages(PAGES_PER_CONTEXT_ARRAY);

    for (int i = 0; i < MAX_CONTEXTS_PER_ARRAY; i++) // todo: we should allocate these on demand
    {
        fbContexts[i] = pmmPage();
        inputContexts[i] = pmmPage();
    }

    fbIndex = 0;
    inputIndex = 0;
}

void drvExit(uint32_t drv)
{
    // clear contexts used by the drv
    for (int i = 0; i < DRV_MAX_CONTEXTS; i++)
    {
        if (fbContexts[i]->pid == drv)
            zero(fbContexts[i], sizeof(drv_context_fb_t));

        if (inputContexts[i]->pid == drv)
            zero(inputContexts[i], sizeof(drv_context_input_t));
    }

    idtClearRedirect(drv);
    logInfo("drv: %s exits", schedGet(drv)->name);
}

void drvUpdateReference(uint32_t type, void *context)
{
    switch (type)
    {
    case DRV_TYPE_FB:
        memcpy(&fbReference, context, sizeof(drv_context_fb_t));
        for (int i = 0; i < fbIndex; i++)
        {
            fbContexts[i]->requestedXres = fbReference.requestedXres;
            fbContexts[i]->requestedYres = fbReference.requestedYres;
        }

        break;

    default:
        break;
    }
}

void *drvRegister(uint32_t drv, uint32_t type)
{
    // return a new context
    logInfo("drv: %s registers as type %d driver", schedGet(drv)->name, type);

    switch (type)
    {

    case DRV_TYPE_FB:
    {
        if (fbIndex == DRV_MAX_CONTEXTS) // hell nah man (todo: reallocate every time)
            return NULL;

        // generate the context using the reference
        fbContexts[fbIndex]->pid = drv;
        fbContexts[fbIndex]->currentXres = framebufferGet().width;
        fbContexts[fbIndex]->currentYres = framebufferGet().height;
        fbContexts[fbIndex]->requestedXres = fbReference.requestedXres;
        fbContexts[fbIndex]->requestedYres = fbReference.requestedYres;
        fbContexts[fbIndex]->base = NULL;

        return fbContexts[fbIndex++];
        break;
    }

    case DRV_TYPE_INPUT:
    {
        if (inputIndex == DRV_MAX_CONTEXTS) // hell nah man (todo: reallocate every time)
            return NULL;

        inputContexts[inputIndex]->pid = drv;
        return inputContexts[inputIndex++];
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
        for (int i = 0; i < fbIndex; i++)
        {
            if (fbContexts[i]->pid && fbContexts[i]->currentXres == fbReference.requestedXres && fbContexts[i]->currentYres == fbReference.requestedYres)
                return fbContexts[i];
        }

        // return first valid if we didn't find one
        for (int i = 0; i < fbIndex; i++)
        {
            if (fbContexts[i]->pid)
                return fbContexts[i];
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
        *contexts = fbContexts;
        *size = fbIndex;
        break;
    }

    case DRV_TYPE_INPUT:
    {
        *contexts = inputContexts;
        *size = inputIndex;
        break;
    }

    default:
        break;
    }
}