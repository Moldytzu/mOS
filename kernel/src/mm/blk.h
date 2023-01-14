#pragma once
#include <misc/utils.h>

#define BLK_ALIGNMENT 16

pstruct
{
    uint16_t size;
    bool free;
    void *prev;
    void *next;
}
blk_header_t;

void blkInit();
void *blkBlock(size_t size);
void blkDeallocate(void *blk);
void *blkReallocate(void *blk, size_t size);
void blkMerge();