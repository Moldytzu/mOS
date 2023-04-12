#include <mm/pmm.h>
#include <mm/vmm.h>
#include <drv/framebuffer.h>
#include <main/panic.h>
#include <cpu/atomic.h>
#include <misc/logger.h>
#include <sched/hpet.h>

uint8_t poolCount = 0;
pmm_pool_t pools[256]; // 256 pools should be enough
bool debug = false;
locker_t pmmLock; // todo: replace this with a per-pool loc

#define PMM_BENCHMARK_SIZE 256 * 48

void pmmBenchmark()
{
    pmmDbgDump();

    // test performance of the allocator
    logInfo("pmm: benchmarking allocation && deallocation");

    uint64_t start = hpetMillis();

    void *addr[PMM_BENCHMARK_SIZE];

    for (int i = 0; i < PMM_BENCHMARK_SIZE; i++) // allocate
        addr[i] = pmmPage();

    uint64_t end = hpetMillis();

    logInfo("pmm: allocation %d KB/ms", (PMM_BENCHMARK_SIZE * 4096 / 1024) / (end - start));

    start = hpetMillis();

    for (int i = 0; i < PMM_BENCHMARK_SIZE; i++) // allocate
        pmmDeallocate(addr[i]);

    end = hpetMillis();

    logInfo("pmm: deallocation %d KB/ms", (PMM_BENCHMARK_SIZE * 4096 / 1024) / (end - start));

    pmmDbgDump();

    hang();
}

void pmmEnableDBG()
{
    debug = true;
}

void pmmDisableDBG()
{
    debug = false;
}

void poolDump(int i)
{
    pmm_pool_t *pool = &pools[i];

    printks("pool %d: ", i);

    // iterate over the bitmap bytes to display the status of the bits
    for (uint64_t b = 0; b < pool->bitmapBytes * 8; b++)
        if (bmpGet(pool->base, b)) // uncomment this to show only the allocated indices
            printks("%d", bmpGet(pool->base, b) ? 1 : 0);

    printks("\n");
}

void pmmDbgDump()
{
    // display all bits in the bitmaps
    for (int i = 0; i < poolCount; i++)
        poolDump(i);
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
                if (((uint64_t *)pool->base)[i / bitsof(uint64_t)] == UINT64_MAX) // if the qword is all set then skip it (speeds up allocation by a lot)
                {
                    i += bitsof(uint64_t) - 1;
                    continue;
                }

                if (bmpGet(pool->base, i)) // find first available index
                    continue;

                bool found = true;
                for (size_t k = i; k < i + pages; k++)
                {
                    if (bmpGet(pool->base, k)) // we didn't find what we need
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
                    bmpSet(pool->base, k);

                // update metadata
                pool->available -= 4096 * pages;
                pool->used += 4096 * pages;

                release(pmmLock);

                return (void *)((uint64_t)pool->alloc + i * 4096);
            }
        }
    });

#undef bits

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
        int i = 0;
        for (;
             i < poolCount &&                                                                              // don't overflow
             !between((uint64_t)page, (uint64_t)pools[i].alloc, (uint64_t)pools[i].alloc + pools[i].size); // make sure the page is in the pool boundaries
             i++)
        {
        }

        uint64_t idx = (uint64_t)(page - pools[i].alloc) / 4096;

        if (!bmpGet(pools[i].base, idx)) // don't deallocate second time
        {
            logWarn("pmm: failed to deallocate");
            release(pmmLock);
            return;
        }

        pools[i].available += 4096;
        pools[i].used -= 4096;

        bmpUnset(pools[i].base, idx); // unset index
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

        if (entry->length < 128 * 1024) // don't bother with very small chunks
            continue;

        // populate the pool metadata
        pmm_pool_t *pool = &pools[poolCount++];
        zero(pool, sizeof(pmm_pool_t));

        pool->bitmapBytes = entry->length / 4096 / 8; // we divide the memory regions in pages (4 KiB chunks) then we will store the availability in a bit in the bitmap

        pool->alloc = (void *)entry->base + pool->bitmapBytes;
        pool->alloc += 4096 - (uint64_t)pool->alloc % 4096;
        pool->alloc += PMM_ALLOC_PADDING;
        
        pool->base = (void *)entry->base;
        pool->available = entry->length - pool->bitmapBytes - PMM_ALLOC_PADDING;
        pool->size = pool->available + pool->used;

        // clear the reserved bytes (holds padding and bitmap information)
        zero(pool->base, (uint64_t)(pool->alloc - pool->base));
    }

    // display the memory pools
    for (int i = 0; i < poolCount; i++)
        logInfo("pmm: [pool %d] {%x -> %x} (%d kb)", i, pools[i].alloc, pools[i].alloc + pools[i].size, pools[i].size / 1024);

    logInfo("pmm: %d MB available", pmmTotal().available / 1024 / 1024);
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