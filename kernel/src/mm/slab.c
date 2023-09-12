#include <mm/slab.h>
#include <mm/pmm.h>
#include <misc/logger.h>

#define OFFSET_BY(slab, x) ((slab_t *)((uint64_t)slab + x))

// creates a new slab list, returns a pointer to it and its total number of pages
slab_t *slabCreate(size_t maxElements, size_t objectSize, size_t *pages)
{
    // todo: see slab.h
    size_t requiredBytes = maxElements * (objectSize + sizeof(slab_t)); // we have to hold in each slab entry header and their object size
    size_t requiredPages = align(requiredBytes, PMM_PAGE) / PMM_PAGE;   // pages to allocate

    *pages = requiredPages; // return number of pages

    logInfo("slab: creating slab with %d elements each having %d bytes (%d bytes, %d pages to allocate)", maxElements, objectSize, requiredBytes, requiredPages);

    slab_t *slabs = pmmPages(requiredPages);

    for (slab_t *slab = slabs; maxElements; maxElements--) // iterate over all items to set required metadata
    {
        slab->free = true;
        slab->index = maxElements;
        slab->size = objectSize;

        slab = OFFSET_BY(slab, objectSize + sizeof(slab_t)); // get next slab
    }

    return slabs;
}

// allocates a new object in a slab list
void *slabAllocate(slab_t *list)
{
    size_t maxElements = list->index;
    size_t objectSize = list->size;

    for (slab_t *slab = list; maxElements; maxElements--)
    {
        // logInfo("%d is %s", slab->index, slab->free ? "free" : "busy");

        if (slab->free) // we can use it
        {
            slab->free = false;
            return OFFSET_BY(slab, sizeof(slab_t));
        }

        slab = OFFSET_BY(slab, objectSize + sizeof(slab_t)); // get next slab
    }

    return NULL;
}

void slabInit()
{
    size_t pages;
    slab_t *slab = slabCreate(10, 32, &pages);
    logInfo("slab: allocated slab at %p with %d pages", slab, pages);

    for (int i = 0; i < 10; i++)
        logInfo("%p", slabAllocate(slab));

    logInfo("%p", slabAllocate(slab));

    while (1)
        ;
}