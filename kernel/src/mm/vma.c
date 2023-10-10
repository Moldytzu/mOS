#include <mm/vma.h>
#include <mm/blk.h>
#include <mm/vmm.h>
#include <misc/logger.h>

#define NEXT_OF(x) ((vma_used_range_t *)x->next)

// fixme: this isn't thread safe!

void dumpRanges(vma_context_t *context)
{
    vma_used_range_t *range = context->usedRanges;

    logDbg(LOG_SERIAL_ONLY, "==============================");
    do
    {
        logDbg(LOG_SERIAL_ONLY, "vma: 0x%p: 0x%p-0x%p next: 0x%p", range, range->start, range->start + range->size, range->next);
        range = range->next;
    } while (range);
}

vma_context_t *vmaCreateContext()
{
    vma_context_t *ctx = blkBlock(sizeof(vma_context_t));
    ctx->usedRanges = blkBlock(sizeof(vma_used_range_t));
    return ctx;
}

void vmaDestroyContext(vma_context_t *context)
{
    for (vma_used_range_t *range = context->usedRanges; range; range = NEXT_OF(range))
        blkDeallocate(range);

    blkDeallocate(context);
}

void vmaMerge(vma_context_t *context)
{
    // merge ranges (try 2 passes)
    for (int i = 0; i < 2; i++)
        for (vma_used_range_t *range = context->usedRanges; range->next; range = NEXT_OF(range))
        {
            vma_used_range_t *next = NEXT_OF(range);

            // check if we can merge forward
            if (range->start + range->size == next->start)
            {
                range->size += next->size;
                range->next = next->next;
                blkDeallocate(next);

                if (!range->next)
                    return;
            }
        }
}

vma_context_t *vmaReserveRange(vma_context_t *context, void *start, uint64_t size)
{
    // check if already reserved
    for (vma_used_range_t *range = context->usedRanges; range; range = NEXT_OF(range))
    {
        if (between((uint64_t)start, range->start, range->start + range->size) && between((uint64_t)start + size, range->start, range->start + range->size))
        {
            logInfo("vma: failed to reserve already reserved range 0x%p-0x%p in %p context", start, start + size, context);
            return context;
        }
    }

    vma_used_range_t *range = context->usedRanges; // point to the first range

    // initialise the first range if it wasn't
    if (range->size == 0)
    {
        range->start = (uint64_t)start;
        range->size = size;
        return context;
    }

    // find a gap where we can reserve while keeping the increasing maner
    for (; range->next; range = NEXT_OF(range))
    {
        vma_used_range_t *nextRange = NEXT_OF(range);

        if (!nextRange)
            break;

        // check if our range is between these ranges
        if (between((uint64_t)start, (range->start + range->size), nextRange->start) && between((uint64_t)start + size, (range->start + range->size), nextRange->start))
            break; // if it is then we will reserve there
    }

    // do the reserving
    volatile vma_used_range_t *newRange = (vma_used_range_t *)blkBlock(sizeof(vma_used_range_t));
    newRange->size = size;
    newRange->start = (uint64_t)start;
    newRange->next = range->next;
    range->next = (vma_used_range_t *)newRange;

    return context;
}

void *vmaAllocatePage(vma_context_t *context)
{
    vma_used_range_t *range = context->usedRanges; // point to the first range
    for (; range->next; range = NEXT_OF(range))
    {
        vma_used_range_t *nextRange = NEXT_OF(range);

        if (!nextRange)
            break;

        // check if it fits between this range
        if (nextRange->start - (range->start + range->size) >= VMM_PAGE)
            break; // if it does break and allocate here
    }

    void *virtualAddress = (void *)(range->start + range->size); // calculate the virtual address
    vmaReserveRange(context, virtualAddress, VMM_PAGE);          // reserve the range
    vmaMerge(context);                                           // merge if possible

    return virtualAddress; // return the address
}

void vmaDeallocateRange(vma_context_t *context, void *start, uint64_t size)
{
    vmaMerge(context); // make sure everything is merged

    for (vma_used_range_t *range = context->usedRanges; range; range = NEXT_OF(range))
    {
        if ((uint64_t)start >= range->start && (uint64_t)start + size <= range->start + range->size)
        {
            // we can split up here
            uint64_t rest = range->size;
            range->size = (uint64_t)start - range->start;
            rest -= range->size;
            rest -= size;

            vma_used_range_t *newRange = blkBlock(sizeof(vma_used_range_t));
            newRange->size = rest;
            newRange->next = range->next;
            newRange->start = (uint64_t)start + size;
            range->next = newRange;

            return;
        }
    }
}

void vmaDeallocatePage(vma_context_t *context, void *page)
{
    vmaDeallocateRange(context, page, VMM_PAGE);

    dumpRanges(context);
}