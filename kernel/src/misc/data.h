#pragma once

static inline __attribute__((always_inline)) void bmpSet(void *bmp, uint64_t idx, uint8_t state)
{
    uint32_t *access = bmp;
    uint64_t arrOffset = idx / (sizeof(uint32_t) * 8);
    uint64_t bitOffset = idx % (sizeof(uint32_t) * 8);
    uint32_t mask = 1 << bitOffset;

    if (state)
        access[arrOffset] |= mask;
    else
        access[arrOffset] &= ~mask;
}

static inline __attribute__((always_inline)) uint8_t bmpGet(void *bmp, uint64_t idx)
{
    uint32_t *access = bmp;
    uint64_t arrOffset = idx / (sizeof(uint32_t) * 8);
    uint64_t bitOffset = idx % (sizeof(uint32_t) * 8);

    return (access[arrOffset] & (1 << bitOffset)) != 0;
}