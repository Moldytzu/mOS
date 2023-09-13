#pragma once
#include <misc/utils.h>

typedef struct
{
    uint16_t objects;    // total number of objects
    uint16_t objectSize; // size in bytes of an object
    uint8_t bitmapBytes; // bytes used by the bitmap
    bool busy;           // busy byte
    spinlock_t lock;     // spinlock

    void *next;    // next cache in chain
    char bitmap[]; // bitmap
} slab_cache_t;

void slabInit();
void slabDeallocate(slab_cache_t *cache, void *ptr);
void *slabAllocate(slab_cache_t *cache);
slab_cache_t *slabCreate(size_t objectSize);