#include <heap.h>
#include <vmm.h>
#include <pmm.h>
#include <panic.h>

struct heap_segment *lastSegment = NULL;
void *end = 0;

void expand(size_t);
void split(struct heap_segment *, size_t);

// initialize the heap
void heapInit()
{
    end = (void *)HEAP_START;
    expand(VMM_PAGE); // expand to a page
}

// expand the heap
void expand(size_t size)
{
    size = align(size, VMM_PAGE); // align the size to page boundary

    struct heap_segment *next = (struct heap_segment *)end; // set new segment to the last address

    // allocate new heap pages
    for (size_t p = 0; p < (size / VMM_PAGE) + 1; p++)
    {
        vmmMap(vmmGetBaseTable(), end, mmAllocatePage(), false, true);
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

    struct heap_segment *currentSegment = (void *)HEAP_START;

    while (currentSegment)
    {
        if (!currentSegment->free || currentSegment->size < size)
        {
            currentSegment = currentSegment->next;
            continue;
        }

        if (currentSegment->size > size)
        {
            split(currentSegment, size + 1); // split the segment at the required size
            currentSegment->free = false;    // mark the segment as busy
            currentSegment->signature = 0x4321;
            return (struct heap_segment *)((uint64_t)currentSegment + (uint64_t)sizeof(struct heap_segment)); // return its content address
        }

        if (currentSegment->size == size)
        {
            currentSegment->free = false; // mark the segment as busy
            currentSegment->signature = 0x4321;
            return (struct heap_segment *)((uint64_t)currentSegment + (uint64_t)sizeof(struct heap_segment)); // return its content address
        }
    }

    expand(size);        // expand the heap
    return malloc(size); // retry
}

// split a segment
void split(struct heap_segment *segment, size_t size)
{
    struct heap_segment *new = (struct heap_segment *)((uint64_t)segment + sizeof(struct heap_segment) + size);
    new->free = true;
    new->size = segment->size - (size + sizeof(struct heap_segment));
    new->next = segment->next;
    new->signature = 0x4321;

    if (segment->next == NULL) // link the segment if the chain is over
        lastSegment = new;

    segment->next = new;  // set new segment
    segment->size = size; // set new size
    segment->signature = 0x4321;
}

// reallocate
void *realloc(void *ptr, size_t size)
{
    void *buffer = malloc(size);

    size_t s = ((struct heap_segment *)((uint64_t)ptr - sizeof(struct heap_segment)))->size;

    if (size < s)
        memcpy8(buffer, ptr, size);
    else
        memcpy8(buffer, ptr, s);

    free(ptr);
    return buffer;
}

// free a segment
void free(void *ptr)
{
    struct heap_segment *seg = ptr - sizeof(struct heap_segment);

    if (seg->signature != 0x4321)
        panick("Misalligned free of a heap segment!");

    seg->free = true; // mark the segment as free
}