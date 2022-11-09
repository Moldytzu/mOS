#include <mm/pmm.h>
#include <mm/vmm.h>
#include <drv/framebuffer.h>
#include <main/panic.h>

uint8_t poolCount = 0;
pmm_pool_t pools[256]; // 256 pools should be enough

void *pmmPage()
{
    for (int i = 0; i < poolCount; i++)
    {
        // find an available pool
        if (pools[i].available < 4096)
            continue;

        pmm_pool_t *pool = &pools[i];

        // iterate over the bitmap bytes to find an available page
        for (; pool->lastBitmapIndex < pool->bitmapBytes; pool->lastBitmapIndex += sizeof(uint64_t))
        {
            for (; pool->lastMaskBit < 64; pool->lastMaskBit++, pool->lastPageIndex++)
            {
                uint64_t mask = 0x8000000000000000 >> pool->lastMaskBit;
                uint64_t *bytes = (uint64_t *)(pool->base + pool->bitmapBytes + pool->lastBitmapIndex);

                if (*bytes & mask) // check against the mask to see if the bit is set
                    continue;

                *bytes |= mask; // set the bit to mark the page as used

                pool->available -= 4096;
                pool->used += 4096;

                return (void *)(pool->alloc + 4096 * pool->lastPageIndex);
            }

            if (pool->lastMaskBit == 64)
                pool->lastMaskBit = 0;
        }
    }

    panick("Out of memory!");

    return NULL;
}

void *pmmPages(uint64_t pages)
{
    uint64_t string = 0;
    uint64_t pageIndex = 0;
    uint64_t allocatedPages = 0;
    void *base = NULL;
    pmm_pool_t *pool;

    for (int i = 0; i < poolCount; i++)
    {
        // find an available pool
        if (pools[i].available < 4096 * pages)
            continue;

        pool = &pools[i];

        // iterate over the bitmap bytes to find an available string of pages
        for (uint64_t b = 0; b < pool->bitmapBytes; b += sizeof(uint64_t))
        {
            for (uint8_t bits = 0; bits < 64; bits++, pageIndex++)
            {
                uint64_t mask = 0x8000000000000000 >> bits;
                uint64_t *bytes = (uint64_t *)(pool->base + b + bits);

                if (*bytes & mask) // check against the mask to see if the page is occupied
                {
                    base = NULL;
                    string = 0;
                    continue;
                }

                if (base == NULL)
                    base = (void *)(pool->alloc + 4096 * pageIndex);

                if (string == pages)
                    goto doReturn;

                string++;
            }
        }
    }

    panick("Out of memory!");

    return NULL;

doReturn:
    pageIndex = 0;

    // set the bits of the bitmap where necessary
    for (volatile uint64_t b = 0; b < pool->bitmapBytes; b += sizeof(uint64_t))
    {
        for (volatile uint8_t bits = 0; bits < 64; bits++, pageIndex++)
        {
            // check if we are in the region
            if ((void *)(pool->alloc + 4096 * pageIndex) <= base)
                continue;

            allocatedPages++;
            *(uint64_t *)(pool->base + b + bits) |= 0x8000000000000000 >> bits;

            pool->available -= 4096;
            pool->used += 4096;

            if (allocatedPages == string)
                return base;
        }
    }

    panick("Out of memory!");

    return NULL;
}

void pmmDeallocate(void *page)
{
    for (int i = 0; i < poolCount; i++)
    {
        pmm_pool_t *pool = &pools[i];
        uint64_t pageIndex = 0;

        // iterate over the bitmap bytes to find an available string of pages
        for (uint64_t b = 0; b < pool->bitmapBytes; b += sizeof(uint64_t))
        {
            for (uint8_t bits = 0; bits < 64; bits++, pageIndex++)
            {
                if ((void *)(pool->alloc + 4096 * pageIndex) != page)
                    continue;

                uint64_t mask = 0x8000000000000000 >> bits;
                uint64_t *bytes = (uint64_t *)(pool->base + b + bits);
                *bytes &= ~mask; // unset the byte

                pool->available += 4096;
                pool->used -= 4096;
                return;
            }
        }
    }
}

void pmmDeallocatePages(void *page, uint64_t count)
{
    for (int i = 0; i < poolCount; i++)
    {
        pmm_pool_t *pool = &pools[i];
        uint64_t pageIndex = 0;

        // iterate over the bitmap bytes to find an available string of pages
        for (uint64_t b = 0; b < pool->bitmapBytes; b += sizeof(uint64_t))
        {
            for (uint8_t bits = 0; bits < 64; bits++, pageIndex++)
            {
                if ((void *)(pool->alloc + 4096 * pageIndex) <= page)
                    continue;

                if (count-- == 0)
                    return;

                uint64_t mask = 0x8000000000000000 >> bits;
                uint64_t *bytes = (uint64_t *)(pool->base + b + bits);
                *bytes &= ~mask; // unset the byte

                pool->available += 4096;
                pool->used -= 4096;
            }
        }
    }
}

void pmmInit()
{
    // get the memory map
    struct limine_memmap_response *map = bootloaderGetMemoryMap();

    memset(pools, 0, sizeof(pools));

    // iterate over the entries
    for (int i = 0; i < map->entry_count; i++)
    {
        struct limine_memmap_entry *entry = map->entries[i];

        // find the usable memory regions
        if (entry->type != LIMINE_MEMMAP_USABLE)
            continue;

        // populate the pool metadata
        pmm_pool_t *pool = &pools[poolCount++];
        memset(pool, 0, sizeof(pmm_pool_t));

        pool->bitmapBytes = entry->length / 4096 / 8; // we divide the memory regions in pages (4 KiB chunks) then we will store the availability in a bit in the bitmap
        pool->alloc = (void *)align((void *)entry->base + pool->bitmapBytes, 4096);
        pool->base = (void *)entry->base;
        pool->available = entry->length - pool->bitmapBytes;
    }

    bootloaderMove();
}

pmm_pool_t pmmTotal()
{
    pmm_pool_t total;

    memset(&total, 0, sizeof(pmm_pool_t));

    for (int i = 0; pools[i].base != NULL; i++) // loop thru each pool
    {
        total.available += pools[i].available; // add each useful property
        total.used += pools[i].used;
        total.bitmapBytes += pools[i].bitmapBytes;
    }
    
    total.lastPageIndex = poolCount + 1; // use pageIndex to set the pool count

    return total; // and return the total
}