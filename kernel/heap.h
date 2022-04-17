#pragma once
#include <utils.h>

#define HEAP_START 0x9000000000

struct pack heap_segment
{
    bool free;
    size_t size;
    struct heap_segment *next;
    struct heap_segment *last;
};

void heapInit();
void free(void *ptr);
void *malloc(size_t size);
void *realloc(void *ptr, size_t size);