#pragma once
#include <stdbool.h>

typedef uint64_t spinlock_t;

void atomicWrite(void *address, uint64_t val);
bool atomicRelease(spinlock_t *locker);
void atomicAquire(spinlock_t *locker);

// hint spinlock
static inline __attribute__((always_inline)) void pause()
{
    asm volatile("pause");
}