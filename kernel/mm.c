#include <mm.h>
#include <framebuffer.h>

struct mm_pool pools[0xFFFF]; // 16k pools should be enough

struct stivale2_struct_tag_memmap *map;

void mmDeallocatePagePool(struct mm_pool *pool, void *address)
{
    uint8_t *currentBitmapByte = pool->base;
    size_t bitmapByteIndex = 0, pageIndex = 0;

     while(bitmapByteIndex != pool->bitmapReserved) // loop thru each byte in the bitmap
    {
        for(int j = 0; j < 8; j++, pageIndex++) // increase the page index on each shift of the mask
        {
            if ((void *)(pool->allocableBase + pageIndex * 4096) == address) // check if we indexed the address
            {
                *currentBitmapByte &= ~(0b10000000 >> j);   // unset that bit
                pool->available += 4096;       // increase available memory
                pool->full = false;            // since we deallocated some memory, we can be sure it's not full
                return;                        // return
            }
            pageIndex++; // increase page index
        }
        currentBitmapByte++; // increase the byte index
    }
}

void *mmAllocatePagePool(struct mm_pool *pool)
{
    uint8_t *currentBitmapByte = pool->base;
    size_t bitmapByteIndex = 0, pageIndex = 0;

    while(bitmapByteIndex != pool->bitmapReserved) // loop thru each byte in the bitmap
    {
        for(int j = 0; j < 8; j++, pageIndex++) // increase the page index on each shift of the mask
        {
            register uint8_t mask = 0b10000000 >> j; // create the mask
            if(!(mask & currentBitmapByte[bitmapByteIndex])) // and the mask, not the result. will return true if the page is not allocated
            {
                pool->available -= 4096; // decrement the available bytes
                if(pool->available < 4096) // if there isn't room for any page it means it's full
                    pool->full = true;
                currentBitmapByte[bitmapByteIndex] |= mask; // apply the mask
                return (void *)(pool->allocableBase + pageIndex * 4096); // return the address
            }
        }
        bitmapByteIndex++; // increase the byte index
    }

    pool->full = true; // we're full
    return NULL; // return null
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
    return NULL;
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

    uint16_t idx = 0;
    for (uint64_t i = 0; i < map->entries; i++)
    {
        if (map->memmap[i].type == STIVALE2_MMAP_USABLE && map->memmap[i].length >= 4096) // if the pool of memory is usable take it
        {
            uint16_t index = idx++;
            pools[index].allocableBase = (void *)map->memmap[i].base; // set the base memory address
            pools[index].base = (void *)map->memmap[i].base;
            pools[index].total = map->memmap[i].length; // set the total memory and available memory to the length of the pool
            pools[index].available = map->memmap[i].length;
            pools[index].full = false; // it has available memory
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

        memset64(pools[i].base, 0, (pools[i].bitmapReserved * 8) / 64); // clear all the bytes in the bitmap
    }
}

struct mm_pool *mmGetPools()
{
    return pools;
}