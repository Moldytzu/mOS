#pragma once
#include <misc/utils.h>

typedef struct
{
    uint16_t objects;
    uint16_t objectSize;
    uint8_t bitmapBytes;
    bool busy;
    spinlock_t lock;

    // todo: create a way to link more slab lists together to possibly have unlimited number of elements
    // this could be implemented in such a way we initially allocate one page
    // and when it is full, we set a pointer in this structure to a new slab list with the same properties

    void *next;

    char bitmap[]; // bitmap
} slab_cache_t;

typedef struct
{
    bool busy;
} slab_meta_t;

void slabInit();