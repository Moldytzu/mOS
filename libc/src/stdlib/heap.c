#include <stdlib.h>
#include <sys.h>
#include <stdio.h>

#define align(val, alg) (max(val,alg) + (alg - (max(val,alg) % alg)))
#define alignD(val, alg) (align(val, alg) - alg)
#define HEADER(segment) ((struct heap_segment *)((uint64_t)segment - sizeof(struct heap_segment)))
#define HEAP_START (void *)0xB000000000

struct heap_segment *lastSegment = NULL;
void *end = NULL;
int initialised = -1;

struct heap_segment
{
    uint16_t signature;
    bool free;
    size_t size;
    struct heap_segment *next;
};

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
}

// expand the heap
void expand(size_t size)
{
    puts("a");
    size = alignD(size, 4096); // align the size to page boundary

    struct heap_segment *next = (struct heap_segment *)end; // set new segment to the last address

    puts("b");

    // allocate new heap pages
    for (size_t p = 0; p < (size / 4096) + 1; p++)
    {
        puts("c");
        void *tmp;
        sys_mem(SYS_MEM_ALLOCATE, (uint64_t)&tmp, 0); // allocate
        end += 4096;
    }

    puts("d");
    // generate required metadata
    next->free = true;
    next->next = NULL;
    next->signature = 0x4321;
    next->size = size;

    puts("e");
    if (!lastSegment) // if the last segment is invalid then make it the newly generated segment
        lastSegment = next;
    else if (lastSegment->free == true) // if the last segment is free then extend it's size
        lastSegment->size += size;
    else // else link the segments together
        lastSegment->next = next;

    puts("f");
}

void init()
{
    initialised = 1;
    lastSegment = NULL;
    end = HEAP_START;
    expand(4096);
}

void *malloc(size_t s)
{
    if (s == 0)
        return NULL;

    s = align(s, 16);

    if(initialised < 0)
        init();

    puts("looping\n");

    struct heap_segment *currentSegment = (void *)HEAP_START;

    while (currentSegment)
    {
        if (!currentSegment->free || currentSegment->size < s)
        {
            currentSegment = currentSegment->next;
            continue;
        }

        if (currentSegment->size > s)
        {
            split(currentSegment, s);                                                                         // split the segment at the required size
            currentSegment->free = false;                                                                     // mark the segment as busy
            return (struct heap_segment *)((uint64_t)currentSegment + (uint64_t)sizeof(struct heap_segment)); // return its content address
        }

        if (currentSegment->size == s)
        {
            currentSegment->free = false;                                                                     // mark the segment as busy
            return (struct heap_segment *)((uint64_t)currentSegment + (uint64_t)sizeof(struct heap_segment)); // return its content address
        }
    }

    expand(s);        // expand the heap
    return malloc(s); // retry
}

void free(void *ptr)
{
}