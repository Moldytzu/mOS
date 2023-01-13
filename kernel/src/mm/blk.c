#include <mm/blk.h>
#include <mm/pmm.h>
#include <main/panic.h>

#define HEADER_AT(ptr) ((blk_header_t *)(ptr))
#define CONTENT_OF(hdr) ((void *)((uint64_t)hdr + sizeof(blk_header_t)))

blk_header_t *start = NULL;

blk_header_t *last()
{
    blk_header_t *current = start;

    while (current->next)
        current = current->next;

    return current;
}

void expand()
{
    // allocate a new block
    blk_header_t *newBlock = (blk_header_t *)pmmPage();
    zero(newBlock, 4096);
    newBlock->free = true;
    newBlock->size = 4096 - sizeof(blk_header_t);

    // add it in the chain
    last()->next = newBlock;
}


void dbgDump()
{
    blk_header_t *current = start;

    printks("==\n");
    do 
    {
        printks("%x is %s and holds %d bytes (total %d bytes)\n",current, current->free ? "free" : "busy", current->size, current->size + sizeof(blk_header_t));
        current = current->next;
    } while(current);
    printks("==\n\n");
}

void blkInit()
{
    // create first block
    start = (blk_header_t *)pmmPage();
    zero(start, 4096);
    start->free = true;
    start->size = 4096 - sizeof(blk_header_t);

    dbgDump();

    blkBlock(32);
    blkBlock(32);
    blkBlock(32);
    blkBlock(1024);
    blkBlock(1024);
    blkBlock(1024);
    blkBlock(1024);
    blkBlock(1024);

    dbgDump();

    while(1);

    printk("blk: initialised!\n");
}

void *blkBlock(size_t size)
{
    if (size % BLK_ALIGNMENT != 0)
        size += size % BLK_ALIGNMENT;

    size += BLK_ALIGNMENT; // padding

    // todo: handle this in another way. maybe wrap the pmm?
    if(size >= 4096 - BLK_ALIGNMENT)
        panick("Failed to allocate a block! Too big.");

    size_t internalSize = size + sizeof(blk_header_t);

    blk_header_t *current = start;

    while (current)
    {
        if (current->size == size && current->free)
        {
            current->free = false;
            return CONTENT_OF(current);
        }

        if (current->size >= internalSize && current->free)
        {
            // create new block then add it in the chain
            blk_header_t *newBlock = CONTENT_OF(current) + size;
            newBlock->size = current->size - internalSize;
            newBlock->free = true;
            newBlock->next = current->next;

            current->next = newBlock;
            current->free = false;
            current->size = size;

            return CONTENT_OF(current);
        }

        current = current->next;
    }

    // expand then try again
    expand();
    return blkBlock(size - BLK_ALIGNMENT); // also remove the padding
}

void blkDeallocate(void *blk)
{
    HEADER_AT(blk)->free = true;
}