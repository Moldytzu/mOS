#include <mm.h>
#include <framebuffer.h>

struct mm_pool pools[0xFFFF]; // 16k pools should be enough
uint16_t poolIdx = 0;

struct stivale2_struct_tag_memmap *map;

struct mm_pool info; // memory info

bool loop = false;
void *mmAllocatePage()
{
    while (info.bitmapByte != info.allocableBase) // loop thru all the bytes in the bitmap
    {
        for(int j = 0; j < 8; j++)
        {
            if((0b10000000 >> j) & *info.bitmapByte) // if there is a bit set, it means that there is a page available
            {
                info.available -= 4096; // decrement the available memory by a page
                *info.bitmapByte &= ~(0b10000000 >> j); // set that bit
                loop = false; // indicate that we are done
                return (void*)(info.allocableBase + info.bitmapIndex * 4096); // return the address
            }
            info.bitmapIndex++; // increase page index in memory
        }
        info.bitmapByte++; // increase byte in bitmap
    }

    // if we don't find an available page search again from the begining

    if(loop) // if the second search was unsucessful return null
    {
        loop = false; // reset
        return NULL;
    }

    info.bitmapByte = info.base; // reset byte
    loop = true; // indicate that we are using recursivity
    return mmAllocatePage();
}

void mmDeallocatePage(void *address)
{
    uint8_t *byte = info.base; // byte in the bitmap
    size_t i = 0; // index

    while (byte != info.allocableBase) // loop thru all the bytes in the bitmap
    {
        for(int j = 0; j < 8; j++)
        {
            if((void*)(info.allocableBase + i * 4096) == address) // check if we indexed the address
            {
                *byte |= 0b10000000 >> j; // set that bit
                info.available += 4096; // increase available memory
                return; // return
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
    for(int i = 0xFFFF-1; i; i--)
        memset64(&pools[i],0,5); // 3 uint64_t and 2 pointers

    for(uint64_t i = 0; i < map->entries; i++)
    {
        if(map->memmap[i].type == STIVALE2_MMAP_USABLE)
        {
            pools[poolIdx++].allocableBase = (void*)map->memmap[i].base; // set the base memory address
            pools[poolIdx].base = (void*)map->memmap[i].base;
            pools[poolIdx].total = map->memmap[i].base; // set the total memory and available memory to the length of the pool
            pools[poolIdx].available = map->memmap[i].base;
        }
        
        if (info.total < map->memmap[i].length && map->memmap[i].type == STIVALE2_MMAP_USABLE) // check if this is the longest strip of memory and if it's usable
        {
            info.total = info.available = map->memmap[i].length; // if yes set the total memory and available memory to the length of the strip
            info.base = info.allocableBase = (void*)map->memmap[i].base; // and set the base memory address
        }
    }

    // we need to calculate how many bytes we need for the allocator's bitmap, storing information about 8 pages per byte
    // let x -> usable memory in bytes; bytes = x/(4096*8);
    size_t bytes = info.available / (4096 * 8);
    info.allocableBase += bytes;
    info.available -= bytes;

    // align the allocableBase to 4096
    info.allocableBase = (void*)align((uint64_t)info.allocableBase,4096);

    // assign the start byte 
    info.bitmapByte = info.base;

    memset64(info.base,0xFF,(bytes*8)/64); // fill all the bytes
}

struct mm_pool mmGetInfo()
{
    return info;
}

struct mm_pool *mmGetPools()
{
    return pools;
}