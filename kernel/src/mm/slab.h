#pragma once
#include <misc/utils.h>

typedef struct
{
    uint16_t index;
    uint16_t size;
    bool free;

    // todo: create a way to link more slab lists together to possibly have unlimited number of elements
    // this could be implemented in such a way we initially allocate one page
    // and when it is full, we set a pointer in this structure to a new slab list with the same properties

    uint16_t padding; // padding to add up to 8 bytes
} slab_t;

void slabInit();