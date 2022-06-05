#include <heap.h>
#include <vmm.h>
#include <pmm.h>

struct heap_segment *lastSegment = NULL;
struct heap_segment *currentSegment = NULL;
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
    size += sizeof(struct heap_segment); // count the first segment's header
    size = align(size, VMM_PAGE);        // align the size to a page's size

    size_t pages = size / VMM_PAGE; // calculate how many pages we have to allocate

    struct heap_segment *next = (struct heap_segment *)end; // set next segment to last address

    // allocate next pages
    for (size_t i = 0; i < pages; i++)
    {
        vmmMap(vmmGetBaseTable(), end, mmAllocatePage(), false, true); // map next page
        end += VMM_PAGE;                                               // move to the next page
    }

    if (lastSegment != NULL && lastSegment->free) // expand last segment if it's free
    {
        lastSegment->size += size;
    }
    else // create another if not
    {
        next->size = size - sizeof(struct heap_segment);
        next->free = true;
        next->last = lastSegment;
        next->next = NULL;

        // link the blocks
        if (lastSegment != NULL)
        {
            lastSegment->next = next;
        }
        lastSegment = next;
    }

    if (currentSegment == NULL)
    {
        currentSegment = next;
    }
}

// allocate on the heap
void *malloc(size_t size)
{
    if (size == 0)
        return NULL;

    size = align(size, 0x10);

    while (1) // loop thru each segment
    {
        if (!currentSegment->free)
        {
            if (!currentSegment->next)
                break;                             // if we are at the last segment exit out of loop
            currentSegment = currentSegment->next; // check next segment
            continue;
        }

        if (currentSegment->size > size)
        {
            split(currentSegment, size);  // split this segment at the needed lenght
            currentSegment->free = false; // set as busy
            return (void *)((uint64_t)currentSegment + sizeof(struct heap_segment));
        }
        else if (currentSegment->size == size)
        {
            currentSegment->free = false; // set as busy
            return (void *)((uint64_t)currentSegment + sizeof(struct heap_segment));
        }
    }

    return NULL;
}

// split a segment
void split(struct heap_segment *segment, size_t size)
{
    if (segment->size < size + sizeof(struct heap_segment)) // can't split a small segment at a larger size
        return;

    struct heap_segment *new = (struct heap_segment *)((uint64_t)segment + sizeof(struct heap_segment) + size);
    new->free = true;
    new->size = segment->size - (size + sizeof(struct heap_segment));
    new->next = segment->next;
    new->last = segment;

    if (segment->next == NULL) // link the segment if the chain is over
        lastSegment = new;

    segment->next = new;  // set new segment
    segment->size = size; // set new size
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
    ((struct heap_segment *)((uint64_t)ptr - sizeof(struct heap_segment)))->free = true; // mark the segment as free
}