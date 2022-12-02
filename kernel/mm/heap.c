#include <mm/heap.h>
#include <mm/vmm.h>
#include <mm/pmm.h>
#include <main/panic.h>

#define HEADER(segment) ((struct heap_segment_t *)((uint64_t)segment - sizeof(struct heap_segment_t)))

struct heap_segment_t *lastSegment = NULL;
void *end = 0;

void expand(size_t);
void split(struct heap_segment_t *, size_t);

// initialize the heap
void heapInit()
{
    end = (void *)HEAP_START;
    expand(1); // create first segment

    printk("heap: size %d kb\n", ((uint64_t)end - HEAP_START) / 1024);
}

// expand the heap
void expand(size_t size)
{
    size = alignD(size, VMM_PAGE); // align the size to page boundary

#ifdef K_HEAP_DEBUG
    printks("heap: expanding by %d bytes\n\r", size);
#endif

    struct heap_segment_t *next = (struct heap_segment_t *)end; // set new segment to the last address

    // allocate new heap pages
    for (size_t p = 0; p < (size / VMM_PAGE) + 1; p++)
    {
        vmmMap(vmmGetBaseTable(), end, pmmPage(), false, true);
        end += VMM_PAGE;
    }

    // generate required metadata
    next->free = true;
    next->next = NULL;
    next->signature = 0x4321;
    next->size = size;

    if (!lastSegment) // if the last segment is invalid then make it the newly generated segment
        lastSegment = next;
    else if (lastSegment->free == true) // if the last segment is free then extend it's size
        lastSegment->size += size;
    else // else link the segments together
        lastSegment->next = next;
}

// allocate on the heap
void *malloc(size_t size)
{
    if (size == 0)
        printk("Invalid heap allocation");

    size = align(size, 16);

#ifdef K_HEAP_DEBUG
    printks("heap: allocating %d bytes\n\r", size);
#endif

    struct heap_segment_t *currentSegment = (void *)HEAP_START;

    while (currentSegment)
    {
        if (!currentSegment->free || currentSegment->size < size)
        {
            currentSegment = currentSegment->next;
            continue;
        }

        if (currentSegment->size > size)
        {
            split(currentSegment, size);                                                         // split the segment at the required size
            currentSegment->free = false;                                                        // mark the segment as busy
            return (void *)((uint64_t)currentSegment + (uint64_t)sizeof(struct heap_segment_t)); // return its content address
        }

        if (currentSegment->size == size)
        {
            currentSegment->free = false;                                                        // mark the segment as busy
            return (void *)((uint64_t)currentSegment + (uint64_t)sizeof(struct heap_segment_t)); // return its content address
        }
    }

    expand(size);        // expand the heap
    return malloc(size); // retry
}

// split a segment
void split(struct heap_segment_t *segment, size_t size)
{
    struct heap_segment_t *new = (struct heap_segment_t *)((uint64_t)segment + sizeof(struct heap_segment_t) + size);
    new->free = true;
    new->size = segment->size - (size + sizeof(struct heap_segment_t));
    new->next = segment->next;
    new->signature = 0x4321;

    if (segment->next == NULL) // link the segment if the chain is over
        lastSegment = new;

    segment->next = new;  // set new segment
    segment->size = size; // set new size
}

// reallocate
void *realloc(void *ptr, size_t size)
{
    void *buffer = malloc(size);                    // allocate another buffer
    size_t copySize = min(HEADER(ptr)->size, size); // calculate the required bytes to be copied

    // determine fastest safe block size
    if (copySize % sizeof(uint64_t) == 0)
        memcpy64(buffer, ptr, copySize / sizeof(uint64_t));
    else if (copySize % sizeof(uint32_t) == 0)
        memcpy32(buffer, ptr, copySize / sizeof(uint32_t));
    else if (copySize % sizeof(uint16_t) == 0)
        memcpy16(buffer, ptr, copySize / sizeof(uint16_t));
    else
        memcpy8(buffer, ptr, copySize);

    free(ptr);     // free the old buffer
    return buffer; // return the newly allocated buffer
}

// free a segment
void free(void *ptr)
{
    if (HEADER(ptr)->signature != 0x4321)
        panick("Misalligned free of a heap segment!");

    HEADER(ptr)->free = true; // mark the segment as free

#ifdef K_HEAP_DEBUG
    printks("heap: freeing %d bytes\n\r", HEADER(ptr)->size);
#endif
}