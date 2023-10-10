#pragma once
#include <stdint.h>

#define BMP_ACCESS_TYPE uint32_t
#define BMP_ACCESS_BYTES sizeof(BMP_ACCESS_TYPE)
#define BMP_ACCESS_BITS (BMP_ACCESS_BYTES * 8)
#define BMP_WORD_ALIGNED(i) (i % BMP_ACCESS_BITS == 0)

static inline __attribute__((always_inline)) void bmpSet(void *bmp, uint64_t idx)
{
    BMP_ACCESS_TYPE *word = bmp;
    word += idx / BMP_ACCESS_BITS;
    idx = idx % BMP_ACCESS_BITS;

    *word |= (1UL << idx);
}

static inline __attribute__((always_inline)) void bmpUnset(void *bmp, uint64_t idx)
{
    BMP_ACCESS_TYPE *word = bmp;
    word += idx / BMP_ACCESS_BITS;
    idx = idx % BMP_ACCESS_BITS;

    *word &= ~(1UL << idx);
}

static inline __attribute__((always_inline)) uint64_t bmpGet(void *bmp, uint64_t idx)
{
    BMP_ACCESS_TYPE *word = bmp;
    word += idx / BMP_ACCESS_BITS;
    idx = idx % BMP_ACCESS_BITS;

    return *word & (1UL << idx);
}