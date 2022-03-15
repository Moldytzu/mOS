#include <mm.h>
#include <framebuffer.h>

struct stivale2_struct_tag_memmap *map;

struct mm_info info; // memory info

void mmInit()
{
    map = bootloaderGetMemMap(); // get the map

    for(uint64_t i = 0; i < map->entries; i++)
    {
        if (info.total < map->memmap[i].length && map->memmap[i].type == STIVALE2_MMAP_USABLE) // check if this is the longest strip of memory and if it's usable
        {
            info.total = info.available = map->memmap[i].length; // if yes set the total memory and available memory to the length of the strip
            info.base = (void*)map->memmap[i].base; // and set the base memory address
        }
    }
}

struct mm_info mmGetInfo()
{
    return info;
}