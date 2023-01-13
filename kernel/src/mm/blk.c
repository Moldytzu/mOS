#include <mm/blk.h>
#include <mm/pmm.h>

#define HEADER_AT(ptr) ((blk_header_t *)(ptr))

blk_header_t *start = NULL;

void blkInit()
{
    start = (blk_header_t *)pmmPage();
    start->free = true;
    start->size = 4096 - sizeof(blk_header_t);

    printk("blk: allocated first page!\n");
}

void *blkBlock(size_t size)
{

}

void blkDeallocate(void *blk)
{
    HEADER_AT(blk)->free = true;
}