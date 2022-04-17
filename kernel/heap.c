#include <heap.h>
#include <vmm.h>
#include <pmm.h>

struct heap_segment *lastSegment = NULL;
struct heap_segment *currentSegment = NULL;
size_t totalMem;
size_t freeMem;
size_t usedMem;
void *end = 0;

void expand(size_t);

void heapInit()
{
    end = (void*)HEAP_START;
    expand(VMM_PAGE); // expand to a page
}

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

    totalMem += size + sizeof(struct heap_segment);
    freeMem += size + sizeof(struct heap_segment);
}

void *malloc(size_t size)
{
    return NULL;
}

void *realloc(void *ptr, size_t size)
{
    return NULL;
}

void free(void *ptr)
{
}