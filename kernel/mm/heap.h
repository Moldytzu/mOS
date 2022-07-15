#pragma once
#include <misc/utils.h>

#define HEAP_START 0x9000000000

struct heap_segment
{
    uint16_t signature;
    bool free;
    size_t size;
    struct heap_segment *next;
};

void heapInit();
void free(void *ptr);
void *malloc(size_t size);
void *realloc(void *ptr, size_t size);