#include <mm.h>
#include <framebuffer.h>

struct mm_pool pools[0xFFFF]; // 16k pools should be enough

struct stivale2_struct_tag_memmap *map;

bool loop = false;
void *mmAllocatePage()
{
    while (pools[0].bitmapByte != pools[0].allocableBase) // loop thru all the bytes in the bitmap
    {
        for (int j = 0; j < 8; j++)
        {
            if ((0b10000000 >> j) & *pools[0].bitmapByte) // if there is a bit set, it means that there is a page available
            {
                pools[0].available -= 4096;                                        // decrement the available memory by a page
                *pools[0].bitmapByte &= ~(0b10000000 >> j);                        // set that bit
                loop = false;                                                  // indicate that we are done
                return (void *)(pools[0].allocableBase + pools[0].bitmapIndex * 4096); // return the address
            }
            pools[0].bitmapIndex++; // increase page index in memory
        }
        pools[0].bitmapByte++; // increase byte in bitmap
    }

    // if we don't find an available page search again from the begining

    if (loop) // if the second search was unsucessful return null
    {
        loop = false; // reset
        return NULL;
    }

    pools[0].bitmapByte = pools[0].base; // reset byte
    loop = true;                 // indicate that we are using recursivity
    return mmAllocatePage();
}

void mmDeallocatePage(void *address)
{
    uint8_t *byte = pools[0].base; // byte in the bitmap
    size_t i = 0;              // index

    while (byte != pools[0].allocableBase) // loop thru all the bytes in the bitmap
    {
        for (int j = 0; j < 8; j++)
        {
            if ((void *)(pools[0].allocableBase + i * 4096) == address) // check if we indexed the address
            {
                *byte |= 0b10000000 >> j; // set that bit
                pools[0].available += 4096;   // increase available memory
                return;                   // return
            }
            i++; // increase page index in memory
        }
        byte++; // increase byte in bitmap
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
        if (map->memmap[i].type == STIVALE2_MMAP_USABLE) // if the pool of memory is usable take it
        {
            pools[idx++].allocableBase = (void *)map->memmap[i].base; // set the base memory address
            pools[idx].base = (void *)map->memmap[i].base;
            pools[idx].total = map->memmap[i].base; // set the total memory and available memory to the length of the pool
            pools[idx].available = map->memmap[i].base;
        }
    }

    // we need to calculate how many bytes we need for the allocator's bitmap, storing information about 8 pages per byte (we need to calculate this for each pool)
    // let x -> usable memory in bytes; bytes = x/(4096*8);
    for (int i = 0; pools[i].total != 0xFFFFFFFFFFFFFFFF; i++)
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