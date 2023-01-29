#include <mm/blk.h>
#include <mm/pmm.h>
#include <main/panic.h>

#define HEADER_OF(ptr) ((blk_header_t *)(ptr - sizeof(blk_header_t)))
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

void expand(uint16_t pages)
{
    // allocate a new block
    blk_header_t *newBlock = (blk_header_t *)pmmPages(pages);
    zero(newBlock, 4096 * pages);
    newBlock->free = true;
    newBlock->size = (4096 * pages) - sizeof(blk_header_t);
    newBlock->prev = last();

    // add it in the chain
    last()->next = newBlock;
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
        printks("%x is %s and holds %d bytes (total %d bytes)\n", current, current->free ? "free" : "busy", current->size, current->size + sizeof(blk_header_t));
        current = current->next;
    } while (current);
    printks("==\n\n");
}

void blkInit()
{
    // create first block
    start = (blk_header_t *)pmmPage();
    zero(start, 4096);
    start->signature = BLK_HEADER_SIGNATURE;
    start->free = true;
    start->size = 4096 - sizeof(blk_header_t);
}

void *blkBlock(size_t size)
{
    if (size % BLK_ALIGNMENT != 0)
        size += size % BLK_ALIGNMENT;

    size += BLK_ALIGNMENT; // padding

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
            newBlock->prev = current;
            newBlock->signature = BLK_HEADER_SIGNATURE;

            current->next = newBlock;
            current->free = false;
            current->size = size;

            return CONTENT_OF(current);
        }

        current = current->next;
    }

    // expand then try again
    expand(size / 4096 + 1);
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
        printks("blk: invalid deallocation at %x\n", blk);
        return;
    }

    blk_header_t *header = HEADER_OF(blk);
    header->free = true;

    if (header->next && HEADER_AT(header->next)->free) // we can merge forward
    {
        blk_header_t *next = HEADER_AT(header->next);
        header->size += next->size + sizeof(blk_header_t);
        header->next = next->next;
    }

    if (header->prev && HEADER_AT(header->prev)->free) // we can merge backwards
    {
        blk_header_t *prev = HEADER_AT(header->prev);
        prev->size += header->size + sizeof(blk_header_t);
        prev->free = true;
        prev->next = header->next;
    }
}