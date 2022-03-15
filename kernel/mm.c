#include <mm.h>
#include <framebuffer.h>

struct stivale2_struct_tag_memmap *map;

uint64_t totalMem = 0; // total memory
uint64_t availableMem = 0; // available memory
uint64_t usedMem = 0; // used memory
void *baseMem = 0; // base address of the physical memory

void mmInit()
{
    map = bootloaderGetMemMap(); // get the map

    for(uint64_t i = 0; i < map->entries; i++)
    {
        if (totalMem < map->memmap[i].length && map->memmap[i].type == STIVALE2_MMAP_USABLE) // check if this is the longest strip of memory and if it's usable
        {
            totalMem = availableMem = map->memmap[i].length; // if yes set the total memory and available memory to the length of the strip
            baseMem = (void*)map->memmap[i].base; // and set the base memory address
        }
    }

    // debug info
    framebufferWrite("\n\nstart debug info");
    framebufferWrite("\nmap->entries = ");
    framebufferWrite(to_string(map->entries));
    framebufferWrite("\navailableMem = ");
    framebufferWrite(to_string(availableMem));
    framebufferWrite("\nend debug info\n\n");
}