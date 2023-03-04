#pragma once

#define BMP_ACCESS_SIZE uint32_t
#define BMP_ACCESS_BITS (sizeof(BMP_ACCESS_SIZE) * 8)
#define BMP_WORD_ALIGNED(i) (i % BMP_ACCESS_BITS == 0)

static inline __attribute__((always_inline)) void bmpSet(void *bmp, uint64_t idx, uint8_t state)
{
    BMP_ACCESS_SIZE *access = bmp;
    uint64_t arrOffset = idx / BMP_ACCESS_BITS;
    uint64_t bitOffset = idx % BMP_ACCESS_BITS;
    BMP_ACCESS_SIZE mask = 1 << bitOffset;

    if (state)
        access[arrOffset] |= mask;
    else
        access[arrOffset] &= ~mask;
}

static inline __attribute__((always_inline)) uint8_t bmpGet(void *bmp, uint64_t idx)
{
    BMP_ACCESS_SIZE *access = bmp;
    uint64_t arrOffset = idx / BMP_ACCESS_BITS;
    uint64_t bitOffset = idx % BMP_ACCESS_BITS;

    return (access[arrOffset] & (1 << bitOffset)) != 0;
}