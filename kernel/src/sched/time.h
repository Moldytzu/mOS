#pragma once
#include <misc/utils.h>

#define TIME_SOURCE_NONE 0
#define TIME_SOURCE_HPET 1

#define TIME_NANOS_TO_MILIS(x) (x / 1000000)
#define TIME_NANOS_TO_SECS(x) (TIME_NANOS_TO_MILIS(x) / 1000)
#define TIME_MILIS_TO_SECS(x) (x / 1000)

void timeSleepMilis(uint64_t milis);
void timeSource();
uint64_t timeNanos();