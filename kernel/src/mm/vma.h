#pragma once
#include <misc/utils.h>

pstruct
{
    uint64_t start;
    uint64_t size;

    void *next;
}
vma_used_range_t;

pstruct
{
    vma_used_range_t *usedRanges;
}
vma_context_t;

vma_context_t *vmaCreateContext();
vma_context_t *vmaReserveRange(vma_context_t *context, void *start, uint64_t size);
void *vmaAllocatePage(vma_context_t *context);
void vmaDeallocateRange(vma_context_t *context, void *start, uint64_t size);
void vmaDeallocatePage(vma_context_t *context, void *page);