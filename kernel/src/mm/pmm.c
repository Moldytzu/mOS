#include <mm/pmm.h>
#include <mm/vmm.h>
#include <drv/framebuffer.h>
#include <main/panic.h>
#include <cpu/atomic.h>
#include <misc/logger.h>
#include <sched/hpet.h>

uint8_t poolCount = 0;
pmm_pool_t pools[256]; // 256 pools should be enough

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

    logInfo("pmm: allocation %d KB/ms", (PMM_BENCHMARK_SIZE * PMM_PAGE / 1024) / (end - start));

    start = hpetMillis();

    for (int i = 0; i < PMM_BENCHMARK_SIZE; i++) // allocate
        pmmDeallocate(addr[i]);

    end = hpetMillis();

    logInfo("pmm: deallocation %d KB/ms", (PMM_BENCHMARK_SIZE * PMM_PAGE / 1024) / (end - start + 1));

    pmmDbgDump();

    hang();
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
    for (int i = 0; i < poolCount; i++)
    {
        // find an available pool
        if (pools[i].available < PMM_PAGE * pages)
            continue;

        pmm_pool_t *pool = &pools[i];

        bool retried = false;

    retry:
        lock(pool->lock, {
            for (size_t i = pool->lastAllocatedIndex; i < pool->size / PMM_PAGE; i++)
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

                pool->lastAllocatedIndex = i; // remember the page

                // update metadata
                pool->available -= PMM_PAGE * pages;
                pool->used += PMM_PAGE * pages;

                memsetPage((void *)((uint64_t)pool->alloc + i * PMM_PAGE), 0, pages); // initialise memory

                release(pool->lock);

                return (void *)((uint64_t)pool->alloc + i * PMM_PAGE);
            }
        });

        // try again starting at the beginning
        if (retried)
            break;

        pool->lastAllocatedIndex = 0;
        retried = true;
        goto retry;
    }

    panick("Out of memory!");

    return NULL;
}

void *pmmPage()
{
    return pmmPages(1);
}

void pmmDeallocate(void *page)
{
    int i = 0;
    for (;
         i < poolCount &&                                                                              // don't overflow
         !between((uint64_t)page, (uint64_t)pools[i].alloc, (uint64_t)pools[i].alloc + pools[i].size); // make sure the page is in the pool boundaries
         i++)
    {
    }

    lock(pools[i].lock, {
        uint64_t idx = (uint64_t)(page - pools[i].alloc) / PMM_PAGE;

        if (!bmpGet(pools[i].base, idx)) // don't deallocate second time
        {
            logWarn("pmm: failed to deallocate");
            release(pools[i].lock);
            return;
        }

        pools[i].available += PMM_PAGE;
        pools[i].used -= PMM_PAGE;

        bmpUnset(pools[i].base, idx); // unset index
    });
}

void pmmDeallocatePages(void *page, uint64_t count)
{
    for (size_t i = 0; i < count; i++)
        pmmDeallocate((void *)((uint64_t)page + i * PMM_PAGE));
}

void *pmmReallocate(void *ptr, uint64_t oldSize, uint64_t newSize)
{
    // make sure we don't have null values
    if (!oldSize)
        oldSize++;

    if (!newSize)
        newSize++;

    // catch potential buggy reallocations causing undefined behaviour
    if (oldSize == newSize)
    {
        void *callerAddress = __builtin_extract_return_addr(__builtin_return_address(0));
        logError("pmm: potential buggy reallocation detected at %p! oldSize == newSize == %d", callerAddress, oldSize);
    }

    // allocate a new area
    void *newPages = pmmPages(newSize);

    // copy the contents
    memcpy8(newPages, ptr, min(oldSize, newSize) * PMM_PAGE);

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

#ifdef K_IGNORE_LOW_MEMORY
        if (entry->base < 1 * 1024 * 1024) // don't allocate in legacy real mode addressing space
            continue;
#endif

#ifdef K_IGNORE_SMALL_POOLS
        if (entry->length < 128 * 1024) // don't bother with very small chunks
            continue;
#endif

        // populate the pool metadata
        pmm_pool_t *pool = &pools[poolCount++];
        zero(pool, sizeof(pmm_pool_t));

        pool->bitmapBytes = entry->length / PMM_PAGE / 8 + 2; // we divide the memory regions in pages (4 KiB chunks) then we will calculate the bytes in which we can write (add 2 bytes to be sure we don't go in allocable memory)

        pool->alloc = (void *)entry->base + pool->bitmapBytes; // we will start after the bitmap so after the base
        pool->alloc = align(pool->alloc, PMM_PAGE);            // we make sure that the base allocation pointer is page aligned

        pool->base = (void *)entry->base;
        pool->available = entry->length - pool->bitmapBytes;
        pool->size = pool->available;

        // too small to hold a page
        if (pool->size < PMM_PAGE)
        {
            // skip this entry
            poolCount--;
            continue;
        }

        // align size to page
        if (pool->size % 4096 != 0)
            pool->size -= pool->size % 4096;

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

    for (int i = 0; i < poolCount; i++) // loop thru each pool
    {
        total.available += pools[i].available; // add each useful property
        total.used += pools[i].used;
        total.bitmapBytes += pools[i].bitmapBytes;
    }

    return total; // and return the total
}

pmm_pool_t *pmmPools()
{
    return pools;
}