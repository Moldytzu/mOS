#include <mm.h>
#include <framebuffer.h>

struct mm_pool pools[0xFFFF]; // 16k pools should be enough

struct stivale2_struct_tag_memmap *map;

void mmDeallocatePagePool(struct mm_pool pool, void *address)
{
    uint8_t *byte = pool.base; // byte in the bitmap
    size_t i = 0;              // index

    while (byte != pool.allocableBase) // loop thru all the bytes in the bitmap
    {
        for (int j = 0; j < 8; j++)
        {
            if ((void *)(pool.allocableBase + i * 4096) == address) // check if we indexed the address
            {
                *byte |= 0b10000000 >> j; // set that bit
                pool.available += 4096;   // increase available memory
                return;                   // return
            }
            i++; // increase page index in memory
        }
        byte++; // increase byte in bitmap
    }
}

bool loop = false;
void *mmAllocatePagePool(struct mm_pool pool)
{
    while (pool.bitmapByte != pool.allocableBase) // loop thru all the bytes in the bitmap
    {
        for (int j = 0; j < 8; j++)
        {
            if ((0b10000000 >> j) & *pool.bitmapByte) // if there is a bit set, it means that there is a page available
            {
                pool.available -= 4096;                                        // decrement the available memory by a page
                *pool.bitmapByte &= ~(0b10000000 >> j);                        // set that bit
                loop = false;                                                  // indicate that we are done
                return (void *)(pool.allocableBase + pool.bitmapIndex * 4096); // return the address
            }
            pool.bitmapIndex++; // increase page index in memory
        }
        pool.bitmapByte++; // increase byte in bitmap
    }

    // if we don't find an available page search again from the begining

    if (loop) // if the second search was unsucessful return null
    {
        loop = false; // reset
        return NULL;
    }

    pool.bitmapByte = pool.base; // reset byte
    loop = true;                 // indicate that we are using recursivity
    return mmAllocatePage();
}

void *mmAllocatePage()
{
    for(int i = 0; pools[i].total != UINT64_MAX; i++)
    {
        if(pools[i].available >= 4096) // check if a page is available
            return mmAllocatePagePool(pools[i]);
    }
    return NULL;
}

void mmDeallocatePage(void *address)
{
    for(int i = 0; pools[i].total != UINT64_MAX; i++)
    {
        if((uint64_t)pools[i].allocableBase <= (uint64_t)address && (uint64_t)address <= (uint64_t)pools[i].allocableBase + pools[i].total) // check if the memory address is in the boundries of the pool's physical memory range 
            mmDeallocatePagePool(pools[i],address);
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
        }
    }

    // we need to calculate how many bytes we need for the allocator's bitmap, storing information about 8 pages per byte (we need to calculate this for each pool)
    // let x -> usable memory in bytes; bytes = x/(4096*8);
    for (int i = 0; pools[i].total != UINT64_MAX; i++)
    {
        size_t bytes = pools[i].available / (4096 * 8);
        pools[i].allocableBase += bytes;
        pools[i].available -= bytes;

        // align the allocableBase to 4096
        pools[i].allocableBase = (void *)align((uint64_t)pools[i].allocableBase, 4096);

        // assign the start byte
        pools[i].bitmapByte = pools[i].base;

        memset64(pools[i].base, 0xFF, (bytes * 8) / 64); // clear all the bytes in the bitmap
    }
}

struct mm_pool *mmGetPools()
{
    return pools;
}