#pragma once
#include <misc/utils.h>

pstruct
{
    uint16_t size;
    bool free;
    void *next;
}
blk_header_t;

void blkInit();
void *blkBlock(size_t size);
void blkDeallocate(void *blk);