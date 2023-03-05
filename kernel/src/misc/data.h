#pragma once

#define BMP_ACCESS_TYPE uint32_t
#define BMP_ACCESS_BYTES sizeof(BMP_ACCESS_TYPE)
#define BMP_ACCESS_BITS (BMP_ACCESS_BYTES * 8)
#define BMP_WORD_ALIGNED(i) (i % BMP_ACCESS_BITS == 0)

static inline __attribute__((always_inline)) void bmpSet(void *bmp, uint64_t idx, uint8_t state)
{
    BMP_ACCESS_TYPE *b = bmp;
    if (state)
        b[idx / BMP_ACCESS_BITS] |= 1 << ((BMP_ACCESS_BITS - 1) - (idx % BMP_ACCESS_BITS));
    else
        b[idx / BMP_ACCESS_BITS] &= ~(1 << ((BMP_ACCESS_BITS - 1) - (idx % BMP_ACCESS_BITS)));
}

static inline __attribute__((always_inline)) uint8_t bmpGet(void *bmp, uint64_t idx)
{
    BMP_ACCESS_TYPE *b = bmp;
    return ((b[idx / BMP_ACCESS_BITS]) >> ((BMP_ACCESS_BITS - 1) - (idx % BMP_ACCESS_BITS)) & 1);
}