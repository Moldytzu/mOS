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
        }
    }
}

void *pmmPages(uint64_t pages)
{
}

void pmmDeallocate(void *page)
{
}

void pmmDeallocatePages(void *page, uint64_t count)
{
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

    for (int i = 0; i < poolCount; i++)
        printk("%d: 0x%x -> 0x%x (%d KiB), %d bmp\n", i, pools[i].base, pools[i].available + pools[i].bitmapBytes, pools[i].available, pools[i].bitmapBytes);

    for (int i = 0; i < 100; i++)
        printk("%x ", pmmPage());

    while (1)
        ;
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