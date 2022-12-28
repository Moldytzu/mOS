#include <mm/blk.h>
#include <main/panic.h>

uint64_t *pages[32768]; // make it so we can hold a maximum ~1,2 gb of ram (should be enough, increase this value if necessary)
size_t index = 0;

void dbgBMP();

ifunc void expand() // expand with a page
{
    size_t newIndex = index++;
    if (newIndex >= 32768)
        panick("Failed to increase blk pages!");

    pages[newIndex] = pmmPage(); // allocate new page
    zero(pages[newIndex], 16);   // clear the bitmap
}

void setBitmap(uint64_t *bmp, uint8_t idx, bool value)
{
    uint64_t *b = bmp + 1; // store the address of the bitmap byte we're processing

    if (idx >= 64) // move the pointer to the left-most bytes
    {
        b--;
        idx -= 64;
    }

    uint64_t mask = 0b1 << idx; // create a bit map

    if (value)
        *b |= mask; // set bit
    else
        *b &= ~mask; // reset bit
}

bool getBitmap(uint64_t *bmp, uint8_t idx)
{
    uint64_t *b = bmp + 1; // store the address of the bitmap byte we're processing

    if (idx >= 64) // move the pointer to the left-most bytes
    {
        b--;
        idx -= 64;
    }

    uint64_t mask = 0b1 << idx; // create a bit map

    return *b & mask;
}

void *blkBlock(size_t size)
{
    if (size >= BLK_ALLOCABLE) // return a page allocated by the pmm
        return pmmPages(size / 4096 + 1);

    size = max(size, 32); // make sure we don't allocate a block smaller than 32

    if (size % 32 != 0) // make the size divisible with 32 (to be easier to calculate later on)
        size += size % 32;

    for (int i = 0; i < index; i++)
    {
        uint64_t *page = pages[i];
        if (!page)
            continue;

        uint8_t blocks = size / BLK_SIZE;
        uint8_t largest = 0;
        uint8_t start = 255;

        if (blocks == 1) // no need for a fancy loop for one block
            goto allocOne;

        // compute largest contiguous area
        for (int j = 0; j < BLK_MAX_BLOCKS; j++)
        {
            if (getBitmap(page, j))
            {
                largest = 0;
                start = 255;
                continue;
            }

            if (start == 255)
                start = j;

            if (largest++ == blocks) // we found it
            {
                // mark blocks as busy
                for (int k = start; k < start + blocks; k++)
                    setBitmap(page, k, BLK_BUSY);

                break;
            }
        }

        if (largest < blocks) // we have to find another
            continue;

        goto doReturn;

    allocOne:
        bool found = false;
        for (int j = 0; j < BLK_MAX_BLOCKS; j++)
        {
            if (getBitmap(page, j)) // find a free block
                continue;

            found = true;
            start = j;
            break;
        }

        if (!found)
            continue;

        setBitmap(page, start, BLK_BUSY); // mark it as busy

    doReturn:
        return (void *)((uint64_t)page + 32 + start * BLK_SIZE); // skip the bitmap, padding bytes and the bytes that we shouldn't return now
    }

    // try again
    expand();
    return blkBlock(size);
}

void blkDeallocate(void *address, size_t size)
{
    if ((uint64_t)address % 4096 == 0) // allocated by the pmm but returned by us
    {
        pmmDeallocatePages(address, size / 4096 + 1);
        return;
    }

    size = max(size, 32); // make sure we don't allocate a block smaller than 32

    if (size % 32 != 0) // make the size divisible with 32 (to be easier to calculate later on)
        size += size % 32;

    void *parentPage = (void *)((uint64_t)address & 0xfffffffffffff000); // discard offset in the page to get the parent page
    uint8_t blocks = size / BLK_SIZE;
    uint16_t startIdx = (((uint64_t)address & 0xfff) - 32) / BLK_SIZE; // get start index by reversing the algorith used in blkBlock to get the new block address

    // free the blocks
    for (int i = startIdx; i < startIdx + blocks; i++)
        setBitmap(parentPage, i, BLK_FREE);

    // deallocate the parent page if empty
    int free = 0;
    for (int j = 0; j < BLK_MAX_BLOCKS; j++)
        free += getBitmap(parentPage, j) ? 0 : 1;

    if (free != BLK_MAX_BLOCKS) // not empty
        return;

    for (int i = 0; i < index; i++) // null the entry in the array
    {
        if (pages[index] == parentPage)
        {
            pages[index] = NULL;
            return;
        }
    }

    pmmDeallocate(parentPage); // deallocate
}

void *blkReblock(void *page, size_t oldSize, size_t newSize)
{
    void *new = blkBlock(newSize);
    memcpy(new, page, oldSize);

    blkDeallocate(page, oldSize);
    return new;
}

void dbgBMP()
{
    // display each bitmap bit from every single allocated page
    for (int i = 0; i < index; i++)
    {
        for (int j = 0; j < BLK_MAX_BLOCKS; j++)
            printks("%d", getBitmap(pages[i], j) ? 1 : 0);
        printks("_");
    }
}

void blkInit()
{
    expand(); // expand with a page
}