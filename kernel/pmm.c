#include <pmm.h>
#include <framebuffer.h>
#include <panic.h>

uint16_t poolCount = 0;

struct mm_pool pools[0xFFFF]; // 16k pools should be enough

struct stivale2_struct_tag_memmap *map;

bool mmIsFreePage(struct mm_pool *pool, size_t page)
{
    uint8_t *bitmapBase = pool->base;
    size_t bitmapByteIndex = 0, pageIndex = 0;

    while (bitmapByteIndex != pool->bitmapReserved) // loop thru each byte in the bitmap
    {
        for (int j = 0; j < 8; j++, pageIndex++) // increase the page index on each shift of the mask
        {
            if(pageIndex == page)
            {
                register uint8_t mask = 0b10000000 >> j;  // create the mask
                return !(mask & bitmapBase[bitmapByteIndex]); // apply the mask if we are at the correct page
            }
        }
        bitmapByteIndex++; // increase the byte index
    }

    return false; // return false if we're not finding the page
}

void *mmAllocatePagePoolIndex(struct mm_pool *pool, size_t page)
{
    uint8_t *bitmapBase = pool->base;
    size_t bitmapByteIndex = 0, pageIndex = 0;

    while (bitmapByteIndex != pool->bitmapReserved) // loop thru each byte in the bitmap
    {
        for (int j = 0; j < 8; j++, pageIndex++) // increase the page index on each shift of the mask
        {
            if(pageIndex == page)
            {
                register uint8_t mask = 0b10000000 >> j;   // create the mask
                pool->available -= 4096;    // decrement the available bytes
                pool->used += 4096;         // increase the usage
                if (pool->available < 4096) // if there isn't room for any page it means it's full
                    pool->full = true;
                bitmapBase[bitmapByteIndex] |= mask;                     // apply the mask
                return (void *)(pool->allocableBase + pageIndex * 4096); // return the address
            }
        }
        bitmapByteIndex++; // increase the byte index
    }

    return NULL; // return null if it isn't free
}

void mmDeallocatePagePool(struct mm_pool *pool, void *address)
{
    uint8_t *bitmapBase = pool->base;
    size_t bitmapByteIndex = 0, pageIndex = 0;

    while (bitmapByteIndex != pool->bitmapReserved) // loop thru each byte in the bitmap
    {
        for (int j = 0; j < 8; j++, pageIndex++) // increase the page index on each shift of the mask
        {
            if ((void *)(pool->allocableBase + pageIndex * 4096) == address) // check if we indexed the address
            {
                bitmapBase[bitmapByteIndex] &= ~(0b10000000 >> j); // unset that bit
                pool->available += 4096;                           // increase available memory
                pool->used -= 4096;                                // decrease the usage
                pool->full = false;                                // since we deallocated some memory, we can be sure it's not full
                return;                                            // return
            }
        }
        bitmapByteIndex++; // increase the byte index
    }
}

void *mmAllocatePagePool(struct mm_pool *pool)
{
    while (pool->bitmapByteIndex != pool->bitmapReserved) // loop thru each byte in the bitmap
    {
        for (; pool->bitmapBitIndex < 8; pool->bitmapBitIndex++, pool->pageIndex++) // increase the page index on each shift of the mask
        {
            uint8_t mask = 0b10000000 >> pool->bitmapBitIndex;   // create the mask
            if (!(mask & pool->bitmapBase[pool->bitmapByteIndex])) // and the mask, not the result. will return true if the page is not allocated
            {
                pool->available -= 4096;    // decrement the available bytes
                pool->used += 4096;         // increase the usage
                if (pool->available < 4096) // if there isn't room for any page it means it's full
                    pool->full = true;
                pool->bitmapBase[pool->bitmapByteIndex] |= mask;                     // apply the mask
                return (void *)(pool->allocableBase + pool->pageIndex * 4096); // return the address
            }
        }
        if(pool->bitmapBitIndex == 8) pool->bitmapBitIndex = 0; // reset the bit index if it's 8 (over the limit)
        pool->bitmapByteIndex++; // increase the byte index
    }

    pool->full = true; // we're full
    return NULL;       // return null
}

void *mmAllocatePagesPool(struct mm_pool *pool, size_t pages)
{
    size_t poolPageCount = pool->total / 4096;

    for(size_t base = 0; base < poolPageCount - pages; base++)
    {
        bool free = true;
        for(size_t i = 0; i <= pages; i++)
        {
            if(!mmIsFreePage(pool,i+base))
                free = false;
        }

        if(!free) continue; // continue if the threshold isn't free

        void *baseptr = mmAllocatePagePoolIndex(pool,base); // allocate
        for(size_t i = 1; i < pages; i++)
            mmAllocatePagePoolIndex(pool,base + i); // reserve the pages

        return baseptr; // return
    }

    return NULL; // if we don't find
}

void *mmAllocatePages(size_t pages)
{
    for (int i = 0; pools[i].total != UINT64_MAX; i++)
    {
        if (!pools[i].full) // check if the pool isn't full
        {
            void *baseptr = mmAllocatePagesPool(&pools[i],pages);
            if (baseptr) // if we've got the pages
                return baseptr;
        }
    }

    panick("Out of memory!"); // panic if we're out of memory
    return NULL; // unreachable
}

void *mmAllocatePage()
{
    for (int i = 0; pools[i].total != UINT64_MAX; i++)
    {
        if (!pools[i].full) // check if the pool isn't full
        {
            void *page = mmAllocatePagePool(&pools[i]);
            if (page) // if we've got a page
                return page;
        }
    }

    panick("Out of memory!"); // panic if we're out of memory
    return NULL; // unreachable
}

void mmDeallocatePage(void *address)
{
    for (int i = 0; pools[i].total != UINT64_MAX; i++)
    {
        if ((uint64_t)pools[i].allocableBase <= (uint64_t)address && (uint64_t)address <= (uint64_t)pools[i].allocableBase + pools[i].total) // check if the memory address is in the boundries of the pool's physical memory range
            mmDeallocatePagePool(&pools[i], address);
    }
}

void mmInit()
{
    map = bootloaderGetMemMap(); // get the map

    // clear the pools
    for (int i = 0xFFFF - 1; i; i--)
        memset(&pools[i], 0xFF, sizeof(struct mm_pool));

    for (uint64_t i = 0; i < map->entries; i++)
    {
        if (map->memmap[i].type == STIVALE2_MMAP_USABLE && map->memmap[i].length >= 4096) // if the pool of memory is usable take it
        {
            uint16_t index = poolCount++;
            memset(&pools[index], 0, sizeof(struct mm_pool));
            pools[index].allocableBase = (void *)map->memmap[i].base; // set the base memory address
            pools[index].base = (void *)map->memmap[i].base;
            pools[index].total = map->memmap[i].length; // set the total memory and available memory to the length of the pool
            pools[index].available = map->memmap[i].length;
            pools[index].full = false; // it has available memory
            pools[index].bitmapBase = pools[index].base;
        }
    }

    // we need to calculate how many bytes we need for the allocator's bitmap, storing information about 8 pages per byte (we need to calculate this for each pool)
    // let x -> usable memory in bytes; bytes = x/(4096*8);
    for (int i = 0; pools[i].total != UINT64_MAX; i++)
    {
        pools[i].bitmapReserved = pools[i].available / 8 / 4096;
        pools[i].allocableBase += pools[i].bitmapReserved;
        pools[i].available -= pools[i].bitmapReserved;

        // align the allocableBase to 4096
        pools[i].allocableBase = (void *)align((uint64_t)pools[i].allocableBase, 4096);

        memset(pools[i].base, 0, pools[i].bitmapReserved); // clear all the bytes in the bitmap
    }
}

struct mm_pool *mmGetPools()
{
    return pools;
}

struct mm_pool mmGetTotal()
{
    struct mm_pool total;

    memset(&total, 0, sizeof(struct mm_pool));

    for (int i = 0; pools[i].total != UINT64_MAX; i++) // loop thru each pool
    {
        total.available += pools[i].available; // add each useful property
        total.used += pools[i].used;
        total.total += pools[i].total;
        total.bitmapReserved += pools[i].bitmapReserved;
    }

    total.pageIndex = poolCount + 1; // use pageIndex to set the pool count

    return total; // and return the total
}