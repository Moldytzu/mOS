#pragma once
#include <misc/utils.h>

pstruct
{
    uint64_t mutex;
}
locker_t;

void atomicWrite(void *address, uint64_t val);
bool atomicClearLock(locker_t *locker);
bool atomicLock(locker_t *locker);
bool atomicRelease(locker_t *locker);
void atomicAquire(locker_t *locker);
void atomicAquireCli(locker_t *locker);
