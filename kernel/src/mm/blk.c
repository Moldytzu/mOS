#include <mm/blk.h>
#include <mm/pmm.h>
#include <main/panic.h>
#include <misc/logger.h>

#define HEADER_OF(ptr) ((blk_header_t *)(ptr - sizeof(blk_header_t)))
#define HEADER_AT(ptr) ((blk_header_t *)(ptr))
#define CONTENT_OF(hdr) ((void *)((uint64_t)hdr + sizeof(blk_header_t)))

spinlock_t blkLock;
blk_header_t *start = NULL;
blk_header_t *last = NULL;

void expand(uint16_t pages)
{
    // allocate a new block
    blk_header_t *newBlock = (blk_header_t *)pmmPages(pages);
    newBlock->free = true;
    newBlock->size = (PMM_PAGE * pages);
    newBlock->prev = last;
    newBlock->signature = BLK_HEADER_SIGNATURE;

    // add it in the chain
    last->next = newBlock;
    last = last->next;
}

void blkMerge()
{
    // todo: make this function merge all the contiguous free blocks to reduce fragmentation
    // this function will be called each ~10s (not yet called)
}

void dbgDump()
{
    blk_header_t *current = start;

    printks("==\n");
    do
    {
        printks("%x is %s and holds %d bytes\n", current, current->free ? "free" : "busy", current->size);
        current = current->next;
    } while (current);
    printks("==\n\n");
}

void blkInit()
{
    // create first block
    start = last = (blk_header_t *)pmmPages(BLK_EXPAND_INCREMENT);
    start->signature = BLK_HEADER_SIGNATURE;
    start->free = true;
    start->size = BLK_EXPAND_INCREMENT * PMM_PAGE;
}

void *blkBlock(size_t size)
{
    lock(blkLock, {
        size = align(size, BLK_ALIGNMENT); // do the alignment
        size += BLK_ALIGNMENT;             // padding

        size_t internalSize = size;

        for (blk_header_t *current = start; current; current = current->next)
        {
            if (!current->free)
                continue;

            if (current->size == size)
                current->free = false;
            else if (current->size >= internalSize)
            {
                // create new block then add it in the chain
                blk_header_t *newBlock = CONTENT_OF(current) + size;
                newBlock->size = current->size - internalSize;
                newBlock->free = true;
                newBlock->next = current->next;
                newBlock->prev = current;
                newBlock->signature = BLK_HEADER_SIGNATURE;

                current->next = newBlock;
                current->free = false;
                current->size = size;
            }

            zero(CONTENT_OF(current), size); // initialise memory
            release(blkLock);
            return CONTENT_OF(current);
        }
    });

    // expand then try again
    expand(16);
    return blkBlock(size - BLK_ALIGNMENT); // also remove the padding
}

void *blkReallocate(void *blk, size_t size)
{
    if (!blk)
        return blkBlock(size);

    void *newBlock = blkBlock(size);
    memcpy(newBlock, blk, HEADER_OF(blk)->size > size ? size : HEADER_OF(blk)->size);
    blkDeallocate(blk);
    return newBlock;
}

void blkDeallocate(void *blk)
{
    if (!blk || HEADER_OF(blk)->signature != BLK_HEADER_SIGNATURE)
    {
        logWarn("blk: invalid deallocation at %x", blk);
        return;
    }

    lock(blkLock, {
        blk_header_t *header = HEADER_OF(blk);
        header->free = true;
        header->signature = BLK_HEADER_SIGNATURE;

        // fixme: here we potentially waste a little bit of memory (23 bytes per merge) because we don't take in account the header size

        if (header->next && HEADER_AT(header->next)->free) // we can merge forward
        {
            blk_header_t *next = HEADER_AT(header->next);
            header->size += next->size;
            header->next = next->next;
        }

        if (header->prev && HEADER_AT(header->prev)->free) // we can merge backwards
        {
            blk_header_t *prev = HEADER_AT(header->prev);
            prev->size += header->size;
            prev->free = true;
            prev->next = header->next;
            prev->signature = BLK_HEADER_SIGNATURE;
        }
    });
}