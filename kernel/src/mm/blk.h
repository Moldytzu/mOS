#pragma once
#include <misc/utils.h>

#define BLK_ALIGNMENT 0x10
#define BLK_HEADER_SIGNATURE 0xF00F
#define BLK_EXPAND_INCREMENT 1

typedef struct
{
    uint16_t signature;
    uint32_t size;
    bool free;
    void *prev;
    void *next;
} blk_header_t;

void blkInit();
void *blkBlock(size_t size);
void blkDeallocate(void *blk);
void *blkReallocate(void *blk, size_t size);
void blkMerge();