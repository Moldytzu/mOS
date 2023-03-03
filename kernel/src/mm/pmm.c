#include <mm/pmm.h>
#include <mm/vmm.h>
#include <drv/framebuffer.h>
#include <main/panic.h>
#include <cpu/atomic.h>
#include <misc/logger.h>

uint8_t poolCount = 0;
pmm_pool_t pools[256]; // 256 pools should be enough
bool debug = false;
locker_t pmmLock; // todo: replace this with a per-pool loc

void pmmEnableDBG()
{
    debug = true;
}

void pmmDisableDBG()
{
    debug = false;
}

ifunc bool get(pmm_pool_t *pool, size_t idx)
{
    return pool->bitmap[idx].val;
}

ifunc void set(pmm_pool_t *pool, size_t idx, bool value)
{
    pool->bitmap[idx].val = value;
}

void pmmDbgDump()
{
    lock(pmmLock, {
        // display all bits in the bitmaps
        for (int i = 0; i < poolCount; i++)
        {
            pmm_pool_t *pool = &pools[i];

            printks("pool %d: ", i);

            // iterate over the bitmap bytes to display the status of the bits
            for (uint64_t b = 0; b < pool->bitmapBytes * 8; b++)
                printks("%d", get(pool, b) ? 1 : 0);

            printks("\n");
        }
    });
}

void *pmmPages(uint64_t pages)
{
    lock(pmmLock, {
        for (int i = 0; i < poolCount; i++)
        {
            // find an available pool
            if (pools[i].available < 4096)
                continue;

            pmm_pool_t *pool = &pools[i];

            for (size_t i = 0; i < pool->bitmapBytes * 8; i++)
            {
                if (get(pool, i)) // find first available index
                    continue;

                bool found = true;
                for (size_t k = i; k < i + pages; k++)
                {
                    if (get(pool, k)) // we didn't find what we need
                    {
                        i += k - i; // increment the offset (skips some iterations)
                        found = false;
                        break;
                    }
                }

                if (!found) // nah
                    continue;

                // set the needed bits
                for (size_t k = i; k < i + pages; k++)
                    set(pool, k, true);

                // update metadata
                pool->available -= 4096 * pages;
                pool->used += 4096 * pages;

                release(pmmLock);

                return (void *)((uint64_t)pool->alloc + i * 4096);
            }
        }
    });

    panick("Out of memory!");

    return NULL;
}

void *pmmPage()
{
    return pmmPages(1);
}

void pmmDeallocate(void *page)
{
    lock(pmmLock, {
        for (int i = 0; i < poolCount; i++)
        {
            pmm_pool_t *pool = &pools[i];

            // find the parent pool of the page
            if ((uint64_t)page < (uint64_t)pool->alloc)
                continue;

            uint64_t addressOffset = (uint64_t)page - (uint64_t)pool->alloc; // offset from the base allocation address
            uint64_t index = addressOffset / 4096;                           // calculate the bitmap offset

            if (get(pool, index) == 0) // don't deallocate second time
            {
                printks("pmm: deallocating second time %x\n", page);
                return;
            }

            pool->available += 4096;
            pool->used -= 4096;

            set(pool, index, false); // unset bit
        }
    });
}

void pmmDeallocatePages(void *page, uint64_t count)
{
    for (size_t i = 0; i < count; i++)
        pmmDeallocate((void *)((uint64_t)page + i * 4096));
}

void *pmmReallocate(void *ptr, uint64_t oldSize, uint64_t newSize)
{
    // make sure we don't have null values
    if (!oldSize)
        oldSize++;

    if (!newSize)
        newSize++;

    // allocate a new area
    void *newPages = pmmPages(newSize);

    // copy the contents
    memcpy8(newPages, ptr, min(oldSize, newSize) * 4096);

    // deallocate old page
    pmmDeallocatePages(ptr, oldSize);

    // return new page
    return newPages;
}

void pmmInit()
{
    // get the memory map
    struct limine_memmap_response *map = bootloaderGetMemoryMap();

    zero(pools, sizeof(pools));

    // iterate over the entries
    for (int i = 0; i < map->entry_count; i++)
    {
        struct limine_memmap_entry *entry = map->entries[i];

        // find the usable memory regions
        if (entry->type != LIMINE_MEMMAP_USABLE)
            continue;

        if (entry->length < 1 * 1024 * 1024) // ignore entries lower than 1 mb (don't mess with legacy real mode ram)
            continue;

        // populate the pool metadata
        pmm_pool_t *pool = &pools[poolCount++];
        zero(pool, sizeof(pmm_pool_t));

        pool->bitmapBytes = entry->length / 4096 / 8; // we divide the memory regions in pages (4 KiB chunks) then we will store the availability in a bit in the bitmap
        pool->alloc = (void *)align((void *)entry->base + pool->bitmapBytes, 4096);
        pool->base = pool->bitmap = (void *)entry->base;
        pool->available = entry->length - pool->bitmapBytes;

        // clear the bitmap
        zero(pool->base, pool->bitmapBytes);
    }

    // display the memory pools
    for (int i = 0; i < poolCount; i++)
        logInfo("pmm: [pool %d] {%x -> %x} (%d kb)", i, pools[i].alloc, pools[i].alloc + pools[i].available, pools[i].available / 1024);

    logInfo("pmm: %d MB available", toMB(pmmTotal().available));
}

// todo: make this function return a dedicated structure and not reuse the internal one
pmm_pool_t pmmTotal()
{
    pmm_pool_t total;

    zero(&total, sizeof(pmm_pool_t));

    lock(pmmLock, {
        for (int i = 0; i < poolCount; i++) // loop thru each pool
        {
            total.available += pools[i].available; // add each useful property
            total.used += pools[i].used;
            total.bitmapBytes += pools[i].bitmapBytes;
        }
    });

    return total; // and return the total
}

pmm_pool_t *pmmPools()
{
    return pools;
}