#include <mm.h>

struct stivale2_struct_tag_memmap *map;

void mmInit()
{
    map = bootloaderGetMemMap();
}