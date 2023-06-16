#pragma once
#include <stdbool.h>

typedef struct __attribute__((__packed__))
{
    uint8_t mutex;
} locker_t;

void atomicWrite(void *address, uint64_t val);
bool atomicRelease(locker_t *locker);
void atomicAquire(locker_t *locker);

// hint spinlock
static inline __attribute__((always_inline)) void pause()
{
    asm volatile ("pause");
}