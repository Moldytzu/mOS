#include <mm/blk.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <main/panic.h>
#include <misc/logger.h>

#define HEADER_OF(ptr) ((blk_header_t *)(ptr - sizeof(blk_header_t)))
#define HEADER_AT(ptr) ((blk_header_t *)(ptr))
#define CONTENT_OF(hdr) ((void *)((uint64_t)hdr + sizeof(blk_header_t)))

spinlock_t blkLock; // spinlock for block

blk_header_t *start = (blk_header_t *)0x700000000000;          // first block in chain
blk_header_t *last = (blk_header_t *)0x700000000000;           // last block in chain
uint8_t *lastMappedVirtualAddress = (uint8_t *)0x700000000000; // last virtual address that was mapped

size_t usedPages = 0; // total of all used pages

// expand block chain
void expand(uint16_t pages)
{
    logInfo("blk: expanding to %d pages", usedPages + pages);

    // allocate a new block of pages
    uint8_t *physicalPages = pmmPages(pages);
    blk_header_t *newBlock = (blk_header_t *)lastMappedVirtualAddress;

    // map them
    for (int i = 0; i < pages; i++)
    {
        vmmMapKernel(lastMappedVirtualAddress, physicalPages, VMM_ENTRY_RW);
        lastMappedVirtualAddress += PMM_PAGE;
        physicalPages += PMM_PAGE;
    }

    // generate metadata
    newBlock->free = true;
    newBlock->size = (PMM_PAGE * pages) - sizeof(blk_header_t);
    newBlock->signature = BLK_HEADER_SIGNATURE;

    if (usedPages) // usedPages is zero if we don't have any pages mapped
    {
        // append the block in the chain if there exists any
        newBlock->prev = last;
        last->next = newBlock;
        last = newBlock;
    }

    usedPages += pages; // keep track of used pages

    // check if we can merge
    if (last->prev && HEADER_AT(last->prev)->free)
    {
        blk_header_t *previous = HEADER_AT(last->prev);      // the block that we will merge to
        previous->size += last->size + sizeof(blk_header_t); // add the full size occupied by the whole block
        previous->next = NULL;                               // the previous block is now the last thus it doesn't have any block after it
        last = previous;                                     // remember it's actually last
    }
}

void blkMerge()
{
    // todo: make this function merge all the contiguous free blocks to reduce fragmentation
    // this function will be called each ~10s (not yet called)
}

// dump internal state
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
    expand(BLK_EXPAND_INCREMENT); // generate start block
}

// allocates a new block
void *blkBlock(size_t size)
{
    // align sizes
    size = align(size, BLK_ALIGNMENT);
    size_t fullSize = size + sizeof(blk_header_t); // this will be aligned since blk_header_t is padded internally to a power of 8

    // do the allocation
    lock(blkLock, {
        while (1)
        {
            for (blk_header_t *current = start; current; current = current->next) // iterate over all blocks
            {
                if (!current->free || current->size < size) // skip any block that isn't free and the right size
                    continue;

                if (current->size > fullSize && current->size - fullSize >= sizeof(blk_header_t) + BLK_ALIGNMENT) // split the block if needeed and possible
                {
                    // create new block then add it in the chain
                    blk_header_t *newBlock = CONTENT_OF(current) + size;
                    newBlock->size = current->size - fullSize;
                    newBlock->free = true;
                    newBlock->next = current->next;
                    newBlock->prev = current;
                    newBlock->signature = BLK_HEADER_SIGNATURE;

                    // add it in the chain
                    current->next = newBlock;
                    current->free = false;
                    current->size = size;

                    if (!newBlock->next) // check if it's the last block
                        last = newBlock; // if it is, remember it
                }

                current->free = false; // mark it as busy

                zero(CONTENT_OF(current), size); // initialise memory
                release(blkLock);
                return CONTENT_OF(current); // return a pointer to the contents
            }

            logInfo("blk: expanding to fit %d bytes", size);

            // expand then try again
            expand(size / 4096 + BLK_EXPAND_INCREMENT);
        }
    });
}

// reallocates a block
void *blkReallocate(void *blk, size_t size)
{
    if (!blk)
        return blkBlock(size);

    void *newBlock = blkBlock(size);                        // allocate a new block with the required size
    memcpy(newBlock, blk, min(size, HEADER_OF(blk)->size)); // copy only the minimal size
    blkDeallocate(blk);                                     // deallocate the old block
    return newBlock;                                        // return the new one
}

// deallocates a block
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

        if (header->next && HEADER_AT(header->next)->free) // we can merge forwards
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
            prev->signature = BLK_HEADER_SIGNATURE;
        }
    });
}