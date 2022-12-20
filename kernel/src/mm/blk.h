#pragma once
#include <mm/pmm.h>
#include <misc/utils.h>

#define BLK_SIZE 32

void blkInit();
void *blkBlock(size_t size);
void *blkReblock(void *page, size_t oldSize, size_t newSize);
void blkDeallocate(void *address, size_t size);