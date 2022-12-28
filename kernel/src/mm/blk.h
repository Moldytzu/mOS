#pragma once
#include <mm/pmm.h>
#include <misc/utils.h>

#define BLK_PAGE 4096
#define BLK_BITMAP 32
#define BLK_SIZE 32
#define BLK_ALLOCABLE (BLK_PAGE - BLK_BITMAP)
#define BLK_MAX_BLOCKS ((BLK_PAGE - BLK_BITMAP) / BLK_SIZE)
#define BLK_BUSY true
#define BLK_FREE false

void blkInit();
void *blkBlock(size_t size);
void *blkReblock(void *page, size_t oldSize, size_t newSize);
void blkDeallocate(void *address, size_t size);